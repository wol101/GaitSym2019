var inputFile = File("dino_cpu (5300x5300).png");
var outputFolder = Folder("resized_icons");
var base_name = "icon";

app.preferences.interpolation = ResampleMethod.NEARESTNEIGHBOR;
app.preferences.rulerUnits = Units.PIXELS;

if (!outputFolder.exists) outputFolder.create();

var widths = [1024, 512, 256, 192, 128, 96, 64, 48, 40, 32, 24, 16];
var i;
for (i = 0; i < widths.length; i++)
{
    app.open(inputFile);
    doc = app.activeDocument;
    doc.changeMode(ChangeMode.RGB);

    var width = widths[i];
    var filename = outputFolder + "/" + base_name + width.toString() + "x" + width.toString() + ".png";

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

