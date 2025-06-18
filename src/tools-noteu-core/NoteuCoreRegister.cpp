//
//  noteuCoreRegister.cpp
//  tools-noteu
//
//  Created by Marcos Ortega on 18/3/15.
//  Copyright (c) 2015 Nicaragua Binary S.A. All rights reserved.
//

#include "NoteuCorePrecomp.h"
#include "NoteuCore.h"
#include "NoteuCoreRegister.h"

UI16 AUEditorAtlas::idTipoClase;
UI16 AUEditorMesh::idTipoClase;
UI16 AUIListImages::idTipoClase;
UI16 AUIMenu::idTipoClase;

#ifndef NB_METODO_INICIALIZADOR_CUERPO
#error "Falta inclusion. Aun no se ha definido la macro NB_METODO_INICIALIZADOR_CUERPO."
#endif

NB_METODO_INICIALIZADOR_CUERPO(NoteuCoreRegister){
	PRINTF_INFO("\n\n+++++++++++++ NoteuCoreRegister +++++++++++++++\n\n");
	NBGestorAUObjetos::registrarClase("AUEditorAtlas", &AUEditorAtlas::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUEditorMesh", &AUEditorMesh::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUIListImages", &AUIListImages::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUIMenu", &AUIMenu::idTipoClase);
};