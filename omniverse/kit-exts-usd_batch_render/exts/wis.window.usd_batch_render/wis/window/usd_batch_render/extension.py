import omni.ui
import omni.kit
import carb.events

import os
import time
import asyncio
import pathlib
import enum

class action(enum.Enum):
    load = 1
    render = 2

FILETYPES = [".png", ".tga", ".exr"]

class usd_batch_render(omni.ext.IExt):
    _viewportWindow = None
    _viewport = None
    _output_buffer = ""
    _input_folder_picker = None
    _output_folder_picker = None
    _usd_file_list_index = 0
    _usd_file_list = []
    _last_time = 0
    _next_action = action.load
    _running = False

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
        
        if not self._running:
            return
        
        current_time = time.time()
        if current_time - self._last_time < 5:
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
            capture_extension.options._output_folder = self._ui_output_folder_path.model.get_value_as_string()
            capture_extension.options._file_type = FILETYPES[self._ui_capture_type.model.get_item_value_model().as_int]
            if self._ui_output_file_name.model.get_value_as_string():
                capture_extension.options._file_name = self._ui_output_file_name.model.get_value_as_string()
            else:
                capture_extension.options._file_name = os.path.split(os.path.splitext(self._usd_file_list[self._usd_file_list_index])[0])[1]
            self._output_buffer += 'Saving "%s%s" to "%s"\n' % (capture_extension.options._file_name, capture_extension.options._file_type, capture_extension.options._output_folder)
            self._ui_output_text.model.set_value(self._output_buffer)
            capture_extension.start()
            self._usd_file_list_index += 1
            self._next_action = action.load

    def on_startup(self, ext_id):
        # Get main window viewport.
        self._viewportWindow = omni.ui.Window('Viewport')
        self._viewport = omni.kit.viewport.utility.get_viewport_from_window_name('Viewport')

        self._time = time.time()
        self._visible = True
        self._current_path = pathlib.Path(__file__).parent.resolve()

        # Register for update event.
        self._subs = omni.kit.app.get_app().get_update_event_stream().create_subscription_to_pop(self.on_update)

        # prepare the user interface
        self._window = omni.ui.Window("Batch USD Render",
                                      width=500, height=500,
                                      flags=omni.ui.WINDOW_FLAGS_NO_SCROLLBAR,
                                      dockPreference=omni.ui.DockPreference.RIGHT_TOP)
        with self._window.frame:
            with omni.ui.ScrollingFrame():
                with omni.ui.VStack(spacing=10):
                    omni.ui.Spacer(height=omni.ui.Pixel(1))
                    with omni.ui.HStack(height=0):
                        omni.ui.Label("Input Folder:", height=0, width=omni.ui.Pixel(100))
                        self._ui_input_folder_path = omni.ui.StringField(height=0)
                        omni.ui.Button(omni.ui.get_custom_glyph_code("${glyphs}/folder.svg"),
                                       width=omni.ui.Pixel(60),
                                       mouse_pressed_fn=self._on_input_folder_clicked)

                    with omni.ui.HStack(height=0):
                        omni.ui.Label("Output Folder:", height=0, width=omni.ui.Pixel(100))
                        self._ui_output_folder_path = omni.ui.StringField(height=0)
                        omni.ui.Button(omni.ui.get_custom_glyph_code("${glyphs}/folder.svg"), 
                                       width=omni.ui.Pixel(60),
                                       mouse_pressed_fn=self._on_output_folder_clicked)

                    with omni.ui.HStack(height=0):
                        omni.ui.Label("File Name:", height=0, width=omni.ui.Pixel(100))
                        self._ui_output_file_name = omni.ui.StringField(height=0)
                        self._ui_output_file_name.model.set_value("Capture")
                        self._ui_capture_type = omni.ui.ComboBox(0, *FILETYPES, width=omni.ui.Pixel(60))

                    omni.ui.Separator(height=0)
                    with omni.ui.ScrollingFrame():                        
                        self._ui_output_text = omni.ui.StringField(height=omni.ui.Percent(1000), multiline=True, read_only=True)
                        self._output_buffer += "USB Batch Render starting\n"
                        self._ui_output_text.model.set_value(self._output_buffer)

                    omni.ui.Separator(height=0)
                    with omni.ui.HStack(height=0):
                        omni.ui.Spacer()
                        self._ui_start_button = omni.ui.Button("Start", height=0, width=omni.ui.Pixel(100),
                                                               mouse_pressed_fn=self._on_start_clicked)
    
        self._startup_completed = True


    def on_shutdown(self):
        # Release the update event.
        async def _exitUpdateEvent ():
            self._visible = False
            await omni.kit.app.get_app().next_update_async()
            self._subs = None
            self._vTime = None
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
        input_folder = self._ui_input_folder_path.model.get_value_as_string()
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
        
