//%attributes = {}
C_TEXT:C284($1; $format)

$format:=$1

var $RTF : Blob

$folder:=Folder:C1567(fk desktop folder:K87:19).folder("UnRTF_"+$format)
$folder.delete(Delete with contents:K24:24)
$folder.create()
$options:=New object:C1471("format"; $format)

For each ($file; Folder:C1567(fk resources folder:K87:11).files(fk ignore invisible:K87:22 | fk recursive:K87:7))
	
	If ($file.extension=".rtf")
		$RTF:=$file.getContent()
		$status:=UnRTF($RTF; $options)
		If ($status.success)
			$folder.file($file.name+"."+$format).setText($status.result)
		Else 
			ALERT:C41($status.error)
		End if 
	End if 
	
End for each 