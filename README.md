# Wild behavior in cat robot

## Folder distribution
Nybble: code relative to Arduino code
Communications: program to handle communications on the RaspberryPi
State_machine: code to be executed in C++ in the main computer

Common error sources and possible solutions (in spanish):

* Conectar el programador a NyBoard y cargar WriteInstinct.ino Calibrar la MPU y el offset de los servos
* Cargar el programa Nybble.ino
* Desconectar el programador
* Acceder a la Raspberry Pi, cuando esta es alimentada por la fuente de alimentacio ́n no posee potencia suficiente para conectarse a una pantalla, por lo que se recomienda utilizar un software de manejo remoto como VNC Viewer.
* Comprobar que la Raspberry Pi se encuentra en la misma red que el PC
* Ejecutar el co ́digo secuenciador de movimientos en el PC
* Ejecutar la interfaz de comunicacio ́n sobre la Raspberry Pi
* Como el programa ejecutor de movimientos est ́a ya cargado sobre la NyBoard, se puede comenzar con la interaccio ́n con el robot
* Si el sistema se bloquea en su movimiento, normalmente se soluciona volviendo a calibrar el robot; esto ocurre tras un largo periodo de tiempo sin su utilizacio ́n.
* Si el sistema se bloquea en la fase de comunicaci ́on y se ha comprobado que la IP del servidor y el puerto es correcto, es probable que el fallo se deba al cortafuegos del ordenador. Probablemente al volver a construir el proyecto y aceptarlo como excepcio ́n en la seguridad del ordenador el problema se solucione.
