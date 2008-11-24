// El siguiente bloque ifdef muestra la forma estándar de crear macros que facilitan 
// la exportación de archivos DLL. Todos los archivos de este archivo DLL se compilan con el símbolo Q3BSP_EXPORTS
// definido en la línea de comandos. Este símbolo no se debe definir en ningún proyecto
// que utilice este archivo DLL. De este modo, otros proyectos cuyos archivos de código fuente incluyan el archivo
// interpreta que las funciones Q3BSP_API se importan de un archivo DLL, mientras que este archivo DLL interpreta los símbolos
// definidos en esta macro como si fueran exportados.
/*
#ifdef Q3BSP_EXPORTS
#define Q3BSP_API __declspec(dllexport)
#else
#define Q3BSP_API __declspec(dllimport)
#endif

// Clase exportada de q3bsp.dll
class Q3BSP_API Cq3bsp {
public:
	Cq3bsp(void);
	// TODO: agregar métodos aquí.
};

extern Q3BSP_API int nq3bsp;

Q3BSP_API int fnq3bsp(void);
*/



