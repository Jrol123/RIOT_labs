@echo on
$port =new-Object System.IO.Ports.SerialPort COM3,115200,None,8,one
$port.Open()
pause
:m
    $port.ReadLine()
goto m