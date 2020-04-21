Step 1) Download Arduino IDE
Step 2) Add the board by adding the below given link in File -> Prederences -> Board Manage URL's
	URL: https://dl.espressif.com/dl/package_esp32_index.json
Step 3) Now, it's time to add supporting libraries to the Arduino IDE. For this, copy all the folders from 
	"C:\Users\usrName\path\product-etag-manager\lib" to "C:\Users\usrName\Documents\Arduino\libraries"
Step 4) Once thats done, install a communication driver to get connected with the board with COM PORT.
	Installation File Path: "C:\Users\usrName\path\product-etag-manager\drivers & plugins\CP210x_Universal_Windows_Driver"
	**NOTE: This driver is also available @ https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers
Step 5) Install tool to upload data to ESP32 data block. To install it copy the folder "arduino-esp32fs-plugin-1.0"
	from "C:\Users\usrName\path\product-etag-manager\drivers & plugins" and paste it at this path :
	"C:\Users\usrName\Documents\Arduino\tools"
	**NOTE: This tool is also available @ https://github.com/me-no-dev/arduino-esp32fs-plugin/releases
Step 5) After that, connect the board using USB cable.
Step 6) Finally, select board from tools -> board -> ESP32 Dev Module and COM port from tools -> port -> COM x
Step 7) Simply upload the code.
Step 8) If you have data, as we have in our case upload data by click tools -> ESP32 sketch data upload.