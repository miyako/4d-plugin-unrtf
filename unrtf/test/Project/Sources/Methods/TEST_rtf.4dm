//%attributes = {}
$file:=Folder:C1567(fk desktop folder:K87:19).file("test.rtf")

var $RTF : Blob

$RTF:=$file.getContent()

$options:=New object:C1471("format"; "txt"; "encoding"; "cp1252")

$status:=UnRTF($RTF; $options)

