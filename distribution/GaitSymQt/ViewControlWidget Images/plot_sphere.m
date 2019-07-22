function plot_sphere

markerSize = 18
markerEdgeWidth = 3
markerEdgeColor = [0 0 0]
markerFaceColor = [.49 1 .63]
lineWidth = 5
lineColor = [0.2 0.1 1]

% A sphere with center at the origin may also be specified in spherical coordinates by

% x = rho * cos(theta) * sin(phi)
% y = rho * sin(theta) * sin(phi)
% z = rho * cos(phi)
			
% where theta is an azimuthal coordinate running from 0 to 2 pi (longitude)
% phi is a polar coordinate running from 0 to pi (colatitude)
% and rho is the radius

delta = pi() / 8
count = 0
rho = 1
for hemisphere = 0: 1
    for theta = 0: delta: 2 * pi()
        for phi = 0: delta: pi()
            count = count + 1;
            x(count) = rho * cos(theta) * sin(phi);
            y(count) = rho * sin(theta) * sin(phi);
            if hemisphere == 0
                z(count) = rho * cos(phi);
            else
                z(count) = -1 * rho * cos(phi);
            end
        end
    end
end


for hemisphere = 0: 1
    for phi = 0: delta: pi()
        for theta = 0: delta: 2 * pi()
            count = count + 1;
            x(count) = rho * cos(theta) * sin(phi);
            y(count) = rho * sin(theta) * sin(phi);
            if hemisphere == 0
                z(count) = rho * cos(phi);
            else
                z(count) = -1 * rho * cos(phi);
            end
        end
    end
end

% print out unique values
xp = []
yp = []
zp = []
e = 0.000001
for i = 1: length(z)
    printed = 0;
    for j = 1: length(zp)
        if (abs(xp(j)-x(i))<e && abs(yp(j)-y(i))<e && abs(zp(j)-z(i))<e)
            printed = 1;
            break;
        end
    end
    if (printed == 0)
        disp(sprintf('%.17f %.17f %.17f', x(i), y(i), z(i)))
        xp(end+1) = x(i);
        yp(end+1) = y(i);
        zp(end+1) = z(i);
    end
end
    

figure(1)
clf('reset');
set(gcf, 'Color', 'w');
set(gcf, 'Position', [10, 10, 1000, 1000]);

handles = plot3(x, y, z, '-', x, y, z, 'o')
axis equal
axis([-1.1 1.1 -1.1 1.1 -1.1 1.1])
axis off

set(handles(1), 'LineWidth', lineWidth, 'Color', lineColor)
set(handles(2), 'LineWidth', markerEdgeWidth, 'MarkerEdgeColor', markerEdgeColor, 'MarkerFaceColor', markerFaceColor, 'MarkerSize', markerSize)

view([0, -10, 0])

save2pdf('SideView.pdf',gcf,300)

figure(2)
clf('reset');
set(gcf, 'Color', 'w');
set(gcf, 'Position', [10, 10, 1000, 1000]);

handles = plot3(x, y, z, '-b', x, y, z, 'o')
axis equal
axis([-1.1 1.1 -1.1 1.1 -1.1 1.1])
axis off

set(handles(1), 'LineWidth', lineWidth, 'Color', lineColor)
set(handles(2), 'LineWidth', markerEdgeWidth, 'MarkerEdgeColor', markerEdgeColor, 'MarkerFaceColor', markerFaceColor, 'MarkerSize', markerSize)

view([0, 0, 10])

save2pdf('TopView.pdf',gcf,300)

figure(3)
clf('reset');
set(gcf, 'Color', 'w');
set(gcf, 'Position', [10, 10, 1000, 1000]);

handles = plot3(x, y, z, '-b')
axis equal
axis([-1.1 1.1 -1.1 1.1 -1.1 1.1])
axis off

set(handles(1), 'LineWidth', lineWidth, 'Color', lineColor)

view([0, -10, 0])

save2pdf('SideViewLines.pdf',gcf,300)

figure(4)
clf('reset');
set(gcf, 'Color', 'w');
set(gcf, 'Position', [10, 10, 1000, 1000]);

handles = plot3(x, y, z, 'o')
axis equal
axis([-1.1 1.1 -1.1 1.1 -1.1 1.1])
axis off

set(handles(1), 'LineWidth', markerEdgeWidth, 'MarkerEdgeColor', markerEdgeColor, 'MarkerFaceColor', markerFaceColor, 'MarkerSize', markerSize)

view([0, -10, 0])

save2pdf('SideViewPoints.pdf',gcf,300)

figure(5)
clf('reset');
set(gcf, 'Color', 'w');
set(gcf, 'Position', [10, 10, 1000, 1000]);

handles = plot3(x, y, z, '-b')
axis equal
axis([-1.1 1.1 -1.1 1.1 -1.1 1.1])
axis off

set(handles(1), 'LineWidth', lineWidth, 'Color', lineColor)

view([0, 0, 10])

save2pdf('TopViewLines.pdf',gcf,300)

figure(6)
clf('reset');
set(gcf, 'Color', 'w');
set(gcf, 'Position', [10, 10, 1000, 1000]);

handles = plot3(x, y, z, 'o')
axis equal
axis([-1.1 1.1 -1.1 1.1 -1.1 1.1])
axis off

set(handles(1), 'LineWidth', markerEdgeWidth, 'MarkerEdgeColor', markerEdgeColor, 'MarkerFaceColor', markerFaceColor, 'MarkerSize', markerSize)

view([0, 0, 10])

save2pdf('TopViewPoints.pdf',gcf,300)




%SAVE2PDF Saves a figure as a properly cropped pdf
%
%   save2pdf(pdfFileName,handle,dpi)
%
%   - pdfFileName: Destination to write the pdf to.
%   - handle:  (optional) Handle of the figure to write to a pdf.  If
%              omitted, the current figure is used.  Note that handles
%              are typically the figure number.
%   - dpi: (optional) Integer value of dots per inch (DPI).  Sets
%          resolution of output pdf.  Note that 150 dpi is the Matlab
%          default and this function's default, but 600 dpi is typical for
%          production-quality.
%
%   Saves figure as a pdf with margins cropped to match the figure size.

%   (c) Gabe Hoffmann, gabe.hoffmann@gmail.com
%   Written 8/30/2007
%   Revised 9/22/2007
%   Revised 1/14/2007

function save2pdf(pdfFileName,handle,dpi)

% Verify correct number of arguments
error(nargchk(0,3,nargin));

% If no handle is provided, use the current figure as default
if nargin<1
    [fileName,pathName] = uiputfile('*.pdf','Save to PDF file:');
    if fileName == 0; return; end
    pdfFileName = [pathName,fileName];
end
if nargin<2
    handle = gcf;
end
if nargin<3
    dpi = 150;
end

% Backup previous settings
prePaperType = get(handle,'PaperType');
prePaperUnits = get(handle,'PaperUnits');
preUnits = get(handle,'Units');
prePaperPosition = get(handle,'PaperPosition');
prePaperSize = get(handle,'PaperSize');

% Make changing paper type possible
set(handle,'PaperType','<custom>');

% Set units to all be the same
set(handle,'PaperUnits','inches');
set(handle,'Units','inches');

% Set the page size and position to match the figure's dimensions
paperPosition = get(handle,'PaperPosition');
position = get(handle,'Position');
set(handle,'PaperPosition',[0,0,position(3:4)]);
set(handle,'PaperSize',position(3:4));

% Save the pdf (this is the same method used by "saveas")
print(handle,'-dpdf',pdfFileName,sprintf('-r%d',dpi))

% Restore the previous settings
set(handle,'PaperType',prePaperType);
set(handle,'PaperUnits',prePaperUnits);
set(handle,'Units',preUnits);
set(handle,'PaperPosition',prePaperPosition);
set(handle,'PaperSize',prePaperSize);


