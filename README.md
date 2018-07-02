# Desktop3D-Client
Programa de Escritorio que se encarga de manejar el controlador de Leap Motion para la detección de gestos y rastreo de manos en un entorno virtual.

Los programas relacionados son:
 - Servidor Principal que guarda los documentos de los usuarios https://github.com/vitohuanqui/MultiDesktopServer
 - Aplicativo Android que se conecta al servidor Leap Motion mediante el servidor principal que brinda el acceso al dispotivo. https://github.com/vitohuanqui/MultiDesktopClient
 

## Dependecias
 - Compilador Microsoft Visual C++ 14.0 disponible con Microsoft Visual Studio 2017
 - Windows SDK 8.1 utilizando 
 - Allegro 5.2.0 para la interfaz Gráfica.
 - En el código fuente se encuentran las librerias de LexRis Logic que permiten un mejor control de la librería allegro, junto a algunas herramientas adicionales. https://github.com/chanochambure/LexRisLogicHeaders
 - Ejecutables de cURL para windows disponibles en: http://www.confusedbycode.com/curl/
 - Leap Motion Orion SDK 3.2.0 disponible en:  https://developer.leapmotion.com/releases/leap-motion-orion-320 
