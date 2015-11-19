The STM32F10x Platform.
The stm32_contiki.uvproj created with the keil uvsion 4.74.

Major Changes:
1. In the Keil, do not use the Makefile to build the whole project, so the files all add to the project directory. Besides, when create new header files, need and to the INCLUDE Paths.
	
2. We do not use the Macro: AUTOSTART_PROCESSES to start the application, just manual start in the main file. Like the blink_process.

3. Some of the core files are not included.

