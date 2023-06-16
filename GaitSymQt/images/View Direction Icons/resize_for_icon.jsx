var inputFiles = [File("D:/Unix/cvs/GaitSym2019/GaitSymQt/images/View Direction Icons/View Direction Back 1024x1024.png"),
                  File("D:/Unix/cvs/GaitSym2019/GaitSymQt/images/View Direction Icons/View Direction Bottom 1024x1024.png"),
                  File("D:/Unix/cvs/GaitSym2019/GaitSymQt/images/View Direction Icons/View Direction Front 1024x1024.png"),
                  File("D:/Unix/cvs/GaitSym2019/GaitSymQt/images/View Direction Icons/View Direction Left 1024x1024.png"),
                  File("D:/Unix/cvs/GaitSym2019/GaitSymQt/images/View Direction Icons/View Direction Right 1024x1024.png"),
                  File("D:/Unix/cvs/GaitSym2019/GaitSymQt/images/View Direction Icons/View Direction Top 1024x1024.png")];
var outputFolder = Folder("D:/Unix/cvs/GaitSym2019/GaitSymQt/images/View Direction Icons");
var base_names = ["view_direction_back", "view_direction_bottom", "view_direction_front", "view_direction_left", "view_direction_right", "view_direction_top"];

app.preferences.interpolation = ResampleMethod.NEARESTNEIGHBOR;
app.preferences.rulerUnits = Units.PIXELS;

if (!outputFolder.exists) outputFolder.create();

var widths = [128, 64, 32, 24, 16];
var i;
var j;
for (j = 0; j < inputFiles.length; j++)
{
    for (i = 0; i < widths.length; i++)
    {
        app.open(inputFiles[j]);
        doc = app.activeDocument;
        doc.changeMode(ChangeMode.RGB);

        var width = widths[i];
        var filename = outputFolder + "/" + base_names[j] + width.toString() + "x" + width.toString() + ".png";

        // resize the image
        doc.resizeImage(UnitValue(width,"px"),null,null,ResampleMethod.BICUBICSHARPER);
        doc.resizeCanvas(UnitValue(width,"px"),UnitValue(width,"px"),AnchorPosition.MIDDLECENTER);
        
        // clear the outer 1 pixel
        try
        {
            // could also do this with doc.selection.selectAll() and doc.selection.contract(1) probably
            var margin = 1;
            var selectedRegion = Array(Array(margin,margin),Array(width-margin,margin),Array(width-margin,width-margin),Array(margin,width-margin));
            doc.selection.select(selectedRegion);
            doc.selection.invert();
            doc.selection.cut();
        }
        catch(e)
        {
            // nothing to do - e happens when doc.selection is empty
        }

        var options = new ExportOptionsSaveForWeb();
        options.quality = 100;
        options.format = SaveDocumentType.PNG; // could be .JPEG, TIFF etc
        options.PNG8 = false;
        options.includeProfile = false;
        options.dither = Dither.NONE;
        options.optimized = true;
        options.transparency = true;
        doc.exportDocument(File(filename),ExportType.SAVEFORWEB,options);
        doc.close(SaveOptions.DONOTSAVECHANGES);
    }
}

