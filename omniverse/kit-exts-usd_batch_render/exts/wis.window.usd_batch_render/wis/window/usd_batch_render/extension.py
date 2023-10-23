import omni.ui
import omni.kit
import carb.events
import pxr

import os
import time
import asyncio
import pathlib
import enum
import json
import datetime

class action(enum.Enum):
    load = 1
    render = 2

FILETYPES = [".png", ".tga", ".exr"]
RENDER_PRESETS = ["RAY_TRACE", "PATH_TRACE", "IRAY"]


class usd_batch_render(omni.ext.IExt):
    # I like to define all the class variables
    # grep -o -e 'self._[A-z_]*' extension.py | sort | uniq
    _current_path = None
    _filepicker_selected_folder = None
    _input_folder_picker = None
    _last_time = 0
    _next_action = action.load
    _output_buffer = ''
    _output_folder_picker = None
    _running = False
    _subs = None
    _ui_between_file_delay = None
    _ui_camera_path = None
    _ui_cancel_button = None
    _ui_capture_type = None
    _ui_image_height = None
    _ui_image_width = None
    _ui_input_folder_path = None
    _ui_output_file_name = None
    _ui_output_folder_path = None
    _ui_output_text = None
    _path_trace_spp = None
    _ui_real_time_settle_latency_frames = None
    _ui_render_preset_index = None
    _ui_start_button = None
    _usd_file_list = []
    _usd_file_list_index = 0
    _viewport = None
    _viewportWindow = None
    _window = None
    _preferences = {'input_folder':'',
                    'output_folder':'',
                    'output_file_name':'',
                    'file_type_index':0,
                    'image_width':1920,
                    'image_height':1080,
                    'camera_path':'',
                    'render_preset_index':0,
                    'between_file_delay':5.0,
                    'real_time_settle_latency_frames':0,
                    'path_trace_spp':1
                    }


    # Update event.
    def on_update (self, e: carb.events.IEvent):
        if self._viewportWindow == None:
            return
        
        if (os.path.isdir(self._ui_input_folder_path.model.get_value_as_string()) and
            os.path.isdir(self._ui_output_folder_path.model.get_value_as_string()) and
            self._running is False):
            self._ui_start_button.enabled = True
        else:
            self._ui_start_button.enabled = False            
        
        if self._running:
            self._ui_cancel_button.enabled = True
        else:
            self._ui_cancel_button.enabled = False
            return
        
        current_time = time.time()
        if current_time - self._last_time < self._preferences['between_file_delay']:
            return
        self._last_time = current_time

        if self._next_action is action.load:
            if self._usd_file_list_index < len(self._usd_file_list):
                self._output_buffer += 'Loading "%s"\n' % (self._usd_file_list[self._usd_file_list_index])
                self._ui_output_text.model.set_value(self._output_buffer)
                self._viewport.usd_context.open_stage(self._usd_file_list[self._usd_file_list_index])
                self._next_action = action.render
            else:
                self._running = False
            return

            
        if self._next_action is action.render:
            capture_extension = omni.kit.capture.viewport.CaptureExtension.get_instance()
            capture_extension.options._output_folder = self._preferences['output_folder']
            capture_extension.options._file_type = FILETYPES[self._preferences['file_type_index']]
            if capture_extension.options._file_type == ".exr": capture_extension.options._hdr_output = True
            else: capture_extension.options._hdr_output = False
            if self._preferences['output_file_name']:
                capture_extension.options._file_name = self._preferences['output_file_name']
            else:
                capture_extension.options._file_name = os.path.split(os.path.splitext(self._usd_file_list[self._usd_file_list_index])[0])[1]
            capture_extension.options._width = self._preferences['image_width']
            capture_extension.options._height = self._preferences['image_height']
            capture_extension.options._real_time_settle_latency_frames = self._preferences['real_time_settle_latency_frames']
            capture_extension.options._path_trace_spp = self._preferences['path_trace_spp']
            
            if RENDER_PRESETS[self._preferences['render_preset_index']] == 'PATH_TRACE': capture_extension.options._render_preset = omni.kit.capture.viewport.CaptureRenderPreset.PATH_TRACE
            if RENDER_PRESETS[self._preferences['render_preset_index']] == 'RAY_TRACE': capture_extension.options._render_preset = omni.kit.capture.viewport.CaptureRenderPreset.RAY_TRACE
            if RENDER_PRESETS[self._preferences['render_preset_index']] == 'IRAY': capture_extension.options._render_preset = omni.kit.capture.viewport.CaptureRenderPreset.IRAY

            # set the camera
            stage = self._viewport.usd_context.get_stage()
            non_omniverse_cameras = []
            omniverse_cameras = []
            for prim in pxr.Usd.PrimRange(stage.GetPseudoRoot()):
                if prim.IsA(pxr.UsdGeom.Camera):
                    camera_name = prim.GetPath().pathString
                    if camera_name.startswith('/Omniverse'): omniverse_cameras.append(camera_name)
                    else: non_omniverse_cameras.append(camera_name)
            if self._preferences['camera_path'] in non_omniverse_cameras + omniverse_cameras:
                capture_extension.options._camera = self._preferences['camera_path']
            else:
                if non_omniverse_cameras: capture_extension.options._camera = non_omniverse_cameras[0]
                else: capture_extension.options._camera = omniverse_cameras[0]
                self._output_buffer += 'Setting camera to "%s"\n' % (capture_extension.options._camera)
                self._ui_output_text.model.set_value(self._output_buffer)

            self._output_buffer += 'Saving "%s%s" to "%s"\n' % (capture_extension.options._file_name, capture_extension.options._file_type, capture_extension.options._output_folder)
            self._ui_output_text.model.set_value(self._output_buffer)
            capture_extension.start()
            self._usd_file_list_index += 1
            self._next_action = action.load

    def on_startup(self, ext_id):
        # Get main window viewport.
        self._viewportWindow = omni.ui.Window('Viewport')
        self._viewport = omni.kit.viewport.utility.get_viewport_from_window_name('Viewport')

        self._current_path = pathlib.Path(__file__).parent.resolve()

        # Register for update event.
        self._subs = omni.kit.app.get_app().get_update_event_stream().create_subscription_to_pop(self.on_update)

        # read the preferences
        self._read_preferences()

        # prepare the user interface
        self._window = omni.ui.Window("Batch USD Render",
                                      width=500, height=500,
                                      flags=omni.ui.WINDOW_FLAGS_NO_SCROLLBAR,
                                      dockPreference=omni.ui.DockPreference.RIGHT_TOP)
        with self._window.frame:
            with omni.ui.ScrollingFrame():
                with omni.ui.VStack(spacing=10):
                    omni.ui.Spacer(height=omni.ui.Pixel(1))
                    with omni.ui.HStack(height=0, spacing=10):
                        omni.ui.Label("Input Folder:", height=0, width=omni.ui.Pixel(100))
                        self._ui_input_folder_path = omni.ui.StringField(height=0)
                        self._ui_input_folder_path.model.set_value(self._preferences["input_folder"])
                        omni.ui.Button(omni.ui.get_custom_glyph_code("${glyphs}/folder.svg"),
                                       width=omni.ui.Pixel(60),
                                       mouse_pressed_fn=self._on_input_folder_clicked)

                    with omni.ui.HStack(height=0, spacing=10):
                        omni.ui.Label("Output Folder:", height=0, width=omni.ui.Pixel(100))
                        self._ui_output_folder_path = omni.ui.StringField(height=0)
                        self._ui_output_folder_path.model.set_value(self._preferences["output_folder"])
                        omni.ui.Button(omni.ui.get_custom_glyph_code("${glyphs}/folder.svg"), 
                                       width=omni.ui.Pixel(60),
                                       mouse_pressed_fn=self._on_output_folder_clicked)

                    with omni.ui.HStack(height=0, spacing=10):
                        omni.ui.Label("File Name:", height=0, width=omni.ui.Pixel(100))
                        self._ui_output_file_name = omni.ui.StringField(height=0)
                        self._ui_output_file_name.model.set_value(self._preferences["output_file_name"])
                        self._ui_capture_type = omni.ui.ComboBox(self._preferences["file_type_index"], *FILETYPES, width=omni.ui.Pixel(60))
                    
                    omni.ui.Separator(height=0)
                    with omni.ui.HStack(height=0, spacing=10):
                        omni.ui.Label("Image Width:", height=0, width=omni.ui.Percent(25))
                        self._ui_image_width = omni.ui.IntField(height=0, width=omni.ui.Percent(25))
                        self._ui_image_width.model.set_value(self._preferences["image_width"])
                        omni.ui.Label("Image Height:", height=0, width=omni.ui.Percent(25))
                        self._ui_image_height = omni.ui.IntField(height=0, width=omni.ui.Percent(25))
                        self._ui_image_height.model.set_value(self._preferences["image_height"])

                    with omni.ui.HStack(height=0, spacing=10):
                        omni.ui.Label("Camera Path:", height=0, width=omni.ui.Percent(25))
                        self._ui_camera_path = omni.ui.StringField(height=0, width=omni.ui.Percent(25))
                        self._ui_camera_path.model.set_value(self._preferences["camera_path"])
                        omni.ui.Label("Loading Delay:", height=0, width=omni.ui.Percent(25))
                        self._ui_between_file_delay = omni.ui.FloatField(height=0, width=omni.ui.Percent(25))
                        self._ui_between_file_delay.precision = 1
                        self._ui_between_file_delay.model.set_value(self._preferences["between_file_delay"])
                    
                    omni.ui.Separator(height=0)
                    with omni.ui.HStack(height=0, spacing=10):
                        omni.ui.Label("Settle Latency:", height=0, width=omni.ui.Percent(25))
                        self._ui_real_time_settle_latency_frames = omni.ui.IntField(height=0, width=omni.ui.Percent(25))
                        self._ui_real_time_settle_latency_frames.model.set_value(self._preferences["real_time_settle_latency_frames"])
                        omni.ui.Label("Samples Per Pixel:", height=0, width=omni.ui.Percent(25))
                        self._ui_path_trace_spp = omni.ui.IntField(height=0, width=omni.ui.Percent(25))
                        self._ui_path_trace_spp.model.set_value(self._preferences["path_trace_spp"])

                    with omni.ui.HStack(height=0, spacing=10):
                        omni.ui.Label("Render Preset:", height=0, width=omni.ui.Percent(25))
                        self._ui_render_preset_index = omni.ui.ComboBox(self._preferences["render_preset_index"], *RENDER_PRESETS, width=omni.ui.Percent(25))

                    omni.ui.Separator(height=0)
                    with omni.ui.ScrollingFrame():                        
                        self._ui_output_text = omni.ui.StringField(height=omni.ui.Pixel(10000), width=omni.ui.Pixel(10000), multiline=True, read_only=True)
                        self._output_buffer += 'USB Batch Render starting at %s in\n"%s"\n' % (datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),self._current_path)
                        self._ui_output_text.model.set_value(self._output_buffer)

                    omni.ui.Separator(height=0)
                    with omni.ui.HStack(height=0):
                        omni.ui.Spacer()
                        self._ui_cancel_button = omni.ui.Button("Cancel", height=0, width=omni.ui.Pixel(100), mouse_pressed_fn=self._on_cancel_clicked)
                        self._ui_start_button = omni.ui.Button("Start", height=0, width=omni.ui.Pixel(100), mouse_pressed_fn=self._on_start_clicked)
    


    def on_shutdown(self):
        self._write_preferences()
        # Release the update event.
        async def _exitUpdateEvent ():
            await omni.kit.app.get_app().next_update_async()
            self._subs = None
        asyncio.ensure_future(_exitUpdateEvent())

    def _on_input_folder_clicked(self, x, y, b, m):
        if self._input_folder_picker is None:
            show_collections = ["my-computer"]
            self._input_folder_picker = omni.kit.window.filepicker.FilePickerDialog(
                "Select Folder",
                show_only_collections=show_collections,
                apply_button_label="Select",
                item_filter_fn=lambda item: self._on_input_folder_picker_filter_item(item),
                selection_changed_fn=lambda items: self._on_input_folder_picker_selection_change(items),
                click_apply_handler=lambda filename, dirname: self._on_input_folder_dir_pick(self._input_folder_picker, filename, dirname),
            )
        self._input_folder_picker.set_filebar_label_name("Folder Name: ")
        self._input_folder_picker.refresh_current_directory()
        self._input_folder_picker.show(self._ui_input_folder_path.model.get_value_as_string())

    def _on_input_folder_picker_filter_item(self, item: omni.kit.widget.filebrowser.FileBrowserItem) -> bool:
        if not item or item.is_folder:
            return True
        return False

    def _on_input_folder_picker_selection_change(self, items: [omni.kit.widget.filebrowser.FileBrowserItem] = []):
        last_item = items[-1]
        self._input_folder_picker.set_filename(last_item.name)
        self._filepicker_selected_folder = last_item.path

    def _on_input_folder_dir_pick(self, dialog: omni.kit.window.filepicker.FilePickerDialog, filename: str, dirname: str):
        dialog.hide()
        self._ui_input_folder_path.model.set_value(self._filepicker_selected_folder)

    def _on_output_folder_clicked(self, x, y, b, m):
        if self._output_folder_picker is None:
            show_collections = ["my-computer"]
            self._output_folder_picker = omni.kit.window.filepicker.FilePickerDialog(
                "Select Folder",
                show_only_collections=show_collections,
                apply_button_label="Select",
                item_filter_fn=lambda item: self._on_output_folder_picker_filter_item(item),
                selection_changed_fn=lambda items: self._on_output_folder_picker_selection_change(items),
                click_apply_handler=lambda filename, dirname: self._on_output_folder_dir_pick(self._output_folder_picker, filename, dirname),
            )
        self._output_folder_picker.set_filebar_label_name("Folder Name: ")
        self._output_folder_picker.refresh_current_directory()
        self._output_folder_picker.show(self._ui_output_folder_path.model.get_value_as_string())

    def _on_output_folder_picker_filter_item(self, item: omni.kit.widget.filebrowser.FileBrowserItem) -> bool:
        if not item or item.is_folder:
            return True
        return False

    def _on_output_folder_picker_selection_change(self, items: [omni.kit.widget.filebrowser.FileBrowserItem] = []):
        last_item = items[-1]
        self._input_folder_picker.set_filename(last_item.name)
        self._filepicker_selected_folder = last_item.path

    def _on_output_folder_dir_pick(self, dialog: omni.kit.window.filepicker.FilePickerDialog, filename: str, dirname: str):
        dialog.hide()
        self._ui_output_folder_path.model.set_value(self._filepicker_selected_folder)
    
    def _on_start_clicked(self, x, y, b, m):
        if not self._ui_start_button.enabled:
            return
        self._output_buffer += 'Starting\n'
        self._ui_output_text.model.set_value(self._output_buffer)
        self._write_preferences()
        input_folder = self._preferences['input_folder']
        extensions = ['.usd' , '.usda' , '.usdc']
        self._next_action = action.load
        self._usd_file_list = []
        self._usd_file_list_index = 0
        self._running = True
        if not os.path.isdir(input_folder):
            return
        self._output_buffer += 'Listing "%s"\n' % (input_folder)
        self._ui_output_text.model.set_value(self._output_buffer)
        items = os.listdir(input_folder)
        for item in items:
            if os.path.splitext(item)[1].lower() in extensions:
                path = os.path.join(input_folder, item)
                if os.path.isfile(path):
                    self._usd_file_list.append(path)

    def _on_cancel_clicked(self, x, y, b, m):
        if not self._ui_cancel_button.enabled:
            return
        self._output_buffer += 'Cancelling\n'
        self._ui_output_text.model.set_value(self._output_buffer)
        self._usd_file_list = []
        self._usd_file_list_index = 0
        self._running = False

    def _read_preferences(self):
        prefs_file_name = os.path.join(self._current_path, "usd_batch_render.json")
        if os.path.isfile(prefs_file_name):
            with open(prefs_file_name, 'r', encoding='utf-8') as fp:
                config_data = json.load(fp)
                if isinstance(config_data, dict):
                    for key in self._preferences:
                        if key in config_data and (isinstance(self._preferences[key], type(config_data[key])) or isinstance(config_data[key], type(self._preferences[key]))):
                            self._preferences[key] = config_data[key]

    def _write_preferences(self):
        self._preferences['input_folder'] = self._ui_input_folder_path.model.get_value_as_string()
        self._preferences['output_folder'] = self._ui_output_folder_path.model.get_value_as_string()
        self._preferences['output_file_name'] = self._ui_output_file_name.model.get_value_as_string()
        self._preferences['file_type_index'] = self._ui_capture_type.model.get_item_value_model().as_int
        self._preferences['image_width'] = self._ui_image_width.model.get_value_as_int()
        self._preferences['image_height'] = self._ui_image_height.model.get_value_as_int()
        self._preferences['camera_path'] = self._ui_camera_path.model.get_value_as_string()
        self._preferences['render_preset_index'] = self._ui_render_preset_index.model.get_item_value_model().as_int
        self._preferences['between_file_delay'] = self._ui_between_file_delay.model.get_value_as_float()
        self._preferences['real_time_settle_latency_frames'] = self._ui_real_time_settle_latency_frames.model.get_value_as_int()
        self._preferences['path_trace_spp'] = self._ui_path_trace_spp.model.get_value_as_int()
        prefs_file_name = os.path.join(self._current_path, "usd_batch_render.json")
        with open(prefs_file_name, 'w', encoding='utf-8') as fp:
            json.dump(self._preferences, fp, ensure_ascii=False, sort_keys=True, indent=4, separators=(',', ': '))

