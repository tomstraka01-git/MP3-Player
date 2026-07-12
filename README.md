<p align="center">
  <img src="images/MainAssembly_IMAGE1.png" alt="MainPhoto" style="width: 100%; height: auto; max-width: 100%;">
</p>

# MP3-Player


# Features
- ### Has a 3.5mm audio jack output, you can connect your earphones to it and listen directly.
<p align="center">
  <img src="images/MainAssembly_IMAGE2.png" alt="MainPhoto" width="600">
</p>

- ### Has a micro SD card reader, put the micro SD card inside and listen to your songs.
<p align="center">
  <img src="images/MainAssembly_IMAGE3.png" alt="MainPhoto" width="600">
</p>

- ### Has a micro USB port, so you can flash your own firmware or debug software issues.
<p align="center">
  <img src="images/MainAssembly_IMAGE4.png" alt="MainPhoto" width="600">
</p>

# Electronics
I used a custom PCB, wich takes power from a 3.3V Li-Po 450 mah and passes it through a fast fuse for short circuit protection and diode for reverse connection protection. Then it goes to the rasberry pi pico 2 WH, wich converts the power to stable 3.3V. The MCU reads the micro sd card reader module through spi, and then plays the song into the PCM5102A. It also displays all the info to the oled screen, and checks the five buttons for input. The five buttons have these features:
- Volume Down
- Volume Up
- Previous Song
- Next Song
- Play/Pause

  
![MainPhoto](images/MainAssembly_IMAGE5.png)

![MainPhoto](images/MainAssembly_IMAGE6.PNG)
