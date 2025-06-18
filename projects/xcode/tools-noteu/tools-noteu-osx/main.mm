//
//  main.m
//  tools-noteu
//
//  Created by Marcos Ortega on 18/3/15.
//  Copyright (c) 2015 Nicaragua Binary S.A. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBMngrThreads.h"

int main(int argc, const char * argv[]) {
	int r = 0;
	NBMngrThreads_init();
	NBMngrStructMaps_init();
	//NBSocket_initEngine();
	//NBSocket_initWSA();
	if(!AUApp::inicializarNucleo(AUAPP_BIT_MODULO_RED)){
		PRINTF_ERROR("No se pudo incializar NUCLEO\n");
		return -1;
	} else {
		//------------------------
		//--- Imprimir lista de parametros (informativo)
		//------------------------
		int iParam; for(iParam=0; iParam<argc; iParam++){
			PRINTF_INFO("Parametro exe %d: '%s'\n", iParam, argv[iParam]);
		}
		//------------------------
		//--- Identificar endianes de la aquitectura actual
		//------------------------
		UI16 datoPruebaEndianes = 1; UI8* valoresBytes = (UI8*)&datoPruebaEndianes;
		PRINTF_INFO("El dispositivo es %s-ENDIEN (%d bytes por puntero)\n", (valoresBytes[0]==0 && valoresBytes[1]!=0)?"BIG":"LITTLE", (SI32)sizeof(void*));
		//------------------------
		//--- Inicializar numeros aleatorios
		//------------------------
		srand((unsigned int)time(NULL));
		//------------------------
		//--- Ciclo de App
		//------------------------
		r = NSApplicationMain(argc, argv);
		//
		AUApp::finalizarNucleo();
	}
	//NBSocket_finishWSA();
	//NBSocket_releaseEngine();
	NBMngrStructMaps_release();
	NBMngrThreads_release();
	return r;
}
