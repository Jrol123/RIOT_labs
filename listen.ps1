$port =new-Object System.IO.Ports.SerialPort COM3,115200,None,8,one
$port.Open()
# $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
while(1){$port.ReadLine()}
