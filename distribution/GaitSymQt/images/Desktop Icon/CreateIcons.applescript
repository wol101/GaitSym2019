(* Script for creating App Icon Images:

icon_16x16.png
icon_16x16@2x.png
icon_32x32.png
icon_32x32@2x.png
icon_128x128.png
icon_128x128@2x.png
icon_256x256.png
icon_256x256@2x.png
icon_512x512.png
icon_512x512@2x.png

Author: Heiko Kretschmer
License: Feel free to use it, modify it and spread it.
Complaints: > /dev/null 2>&1
Praises: Contact
*)

on run
	
	try
		(* Let the user choose a original file to down-size and a folder where the new files are saved *)
		tell application "Finder"
			activate
			set fileOriginal to (choose file with prompt "Please choose the original image file:" without multiple selections allowed) as string
			
			set folderIconFiles to (choose folder with prompt "Please choose the folder to save the newly created icon-images into:" without multiple selections allowed) as string
		end tell
		
		tell application "Adobe Photoshop CS6"
			activate
			open file fileOriginal
		end tell
		
		(* Resize the current document and save it *)
		#set pathToDesktop to path to the desktop as text
		tell me to resizeAndSave(folderIconFiles, "icon_", 512, "x", 512, "@", 2, "x.png")
		tell me to resizeAndSave(folderIconFiles, "icon_", 512, "x", 512, "", 1, ".png")
		tell me to resizeAndSave(folderIconFiles, "icon_", 256, "x", 256, "@", 2, "x.png")
		tell me to resizeAndSave(folderIconFiles, "icon_", 256, "x", 256, "", 1, ".png")
		tell me to resizeAndSave(folderIconFiles, "icon_", 128, "x", 128, "@", 2, "x.png")
		tell me to resizeAndSave(folderIconFiles, "icon_", 128, "x", 128, "", 1, ".png")
		tell me to resizeAndSave(folderIconFiles, "icon_", 32, "x", 32, "@", 2, "x.png")
		tell me to resizeAndSave(folderIconFiles, "icon_", 32, "x", 32, "", 1, ".png")
		tell me to resizeAndSave(folderIconFiles, "icon_", 16, "x", 16, "@", 2, "x.png")
		tell me to resizeAndSave(folderIconFiles, "icon_", 16, "x", 16, "", 1, ".png")
		
		(* Close the original file *)
		tell application "Adobe Photoshop CS6"
			close current document saving no
		end tell
		
	on error (e)
		log "Error: " & e
	end try
	
end run


on resizeAndSave(path, prefix, sizex, dingsda, sizey, dingsbums, factor, suffix)
	
	if (factor as integer) = 1 then
		(* Like this: icon_16x16@2x.png *)
		set newFileName to prefix & (sizex as text) & dingsda & (sizey as text) & suffix
	else if (factor as integer) = 2 then
		(* Like this: icon_16x16@2x.png *)
		set newFileName to prefix & (sizex as text) & dingsda & (sizey as text) & dingsbums & factor & suffix
	else
		display dialog "Invalid factor." buttons {"WTF?"}
		return
	end if
	set filePath to path & ":" & newFileName
	
	
	tell application "Adobe Photoshop CS6"
		
		tell current document
			
			resize image width (sizex * factor) height (sizey * factor) resample method bicubic
			save in file filePath as PNG copying yes
			
		end tell
		
	end tell
	
end resizeAndSave
