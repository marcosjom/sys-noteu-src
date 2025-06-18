//
//  AUIMenu.cpp
//  app-refranero
//
//  Created by Marcos Ortega on 14/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#include "NoteuCorePrecomp.h"
#include "AUIMenu.h"
//
	
AUIMenu::AUIMenu() : AUEscenaContenedor()
	, _animObjetos(NULL)
    , _listener(NULL)
    //
    , _optionsFont(NULL)
    , _options(this)
{
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUIMenu")
	//
	_animObjetos		= new(this) AUAnimadorObjetoEscena();
	_limitsSize.ancho	= _limitsSize.alto = 0.0f;
	//
	_optionsFont		= NBGestorTexturas::fuenteTextura("Helvetica", 12, true, false); _optionsFont->retener();
	//
	NBGestorAnimadores::agregarAnimador(this, this);
}

AUIMenu::~AUIMenu(){
	NBGestorAnimadores::quitarAnimador(this);
	//
	if(_animObjetos!=NULL) _animObjetos->liberar(NB_RETENEDOR_THIS); _animObjetos = NULL;
	if(_optionsFont != NULL) _optionsFont->liberar(NB_RETENEDOR_THIS); _optionsFont = NULL;
	//Buttons
	{
		SI32 i; const SI32 count = _options.conteo;
		for(i = 0; i  < count; i++){
			STIMenuOption* button = _options.elemPtr(i);
			button->button->liberar(); button->button = NULL;
			button->separator->liberar(); button->separator = NULL;
		}
		_options.vaciar();
	}
}

//

NBTamano AUIMenu::limitsSize() const {
	return _limitsSize;
}

void AUIMenu::setLimitsSize(const float width, const float height){
	_limitsSize.ancho	= width;
	_limitsSize.alto	= height;
	this->privOrganize();
}

//

IListenerIMenu* AUIMenu::listener() const {
	return _listener;
}

void AUIMenu::setListener(IListenerIMenu* listener){
	_listener = listener;
}

//

void AUIMenu::optionAdd(const char* text){
	STIMenuOption button;
	button.button		= new(this) AUEscenaTexto(_optionsFont);
	button.button->establecerAlineaciones(ENNBTextLineAlignH_Left, ENNBTextAlignV_Base);
	button.button->establecerTexto(text);
	button.button->establecerEscuchadorTouches(this, this);
	button.separator	= new(this) AUEscenaTexto(_optionsFont);
	button.separator->establecerAlineaciones(ENNBTextLineAlignH_Left, ENNBTextAlignV_Base);
	button.separator->establecerTexto("|");
	_options.agregarElemento(button);
	//
	this->agregarObjetoEscena(button.button);
	this->agregarObjetoEscena(button.separator);
	//
	this->privOrganize();
}

void AUIMenu::optionAnimate(const SI32 optionId, const bool animateSuccess){
	if(optionId >= 0 && optionId < _options.conteo){
		STIMenuOption* button = _options.elemPtr(optionId);
		if(animateSuccess){
			button->button->establecerMultiplicadorColor8(0, 255, 0, 255);
			_animObjetos->animarColorMult(button->button, 255, 255, 255, 255, 0.5f);
		} else {
			button->button->establecerMultiplicadorColor8(255, 0, 0, 255);
			_animObjetos->animarColorMult(button->button, 255, 255, 255, 255, 0.5f);
		}
	}
}

void AUIMenu::optionUpdate(const SI32 optionId, const char* text){
	if(optionId >= 0 && optionId < _options.conteo){
		STIMenuOption* button = _options.elemPtr(optionId);
		button->button->establecerTexto(text);
		this->privOrganize();
	}
}

//

void AUIMenu::privOrganize(){
	//Buttons
	const float ascendenteFuente = _optionsFont->ascendenteEscena();
	const float marginH	= 8.0f;
	const float marginV	= 4.0f;
	const float ySup	= 0.0f;
	float xIzq			= 0.0f;
	SI32 i, botonesColocados = 0; const SI32 count = _options.conteo;
	for(i = 0; i  < count; i++){
		STIMenuOption* button = _options.elemPtr(i);
		if(!button->button->visible()){
			button->separator->establecerVisible(false);
		} else {
			const NBCajaAABB cajaBtn		= button->button->cajaAABBLocal();
			const NBCajaAABB cajaSeparador	= button->separator->cajaAABBLocal();
			if(botonesColocados == 0){
				button->separator->establecerVisible(false);
			} else {
				button->separator->establecerVisible(true);
				button->separator->establecerTraslacion(xIzq + marginH - cajaSeparador.xMin, ySup - marginV - ascendenteFuente);
				xIzq += marginH + (cajaSeparador.xMax - cajaSeparador.xMin);
			}
			button->button->establecerVisible(true);
			button->button->establecerTraslacion(xIzq + marginH - cajaBtn.xMin, ySup - marginV - ascendenteFuente);
			xIzq += marginH + (cajaBtn.xMax - cajaBtn.xMin);
			//
			botonesColocados++;
		}
	}
}

//

void AUIMenu::tickAnimacion(float segsTranscurridos){
	//
}

//TOUCHES
void AUIMenu::touchIniciado(STGTouch* touch, const NBPunto &posTouchEscena, AUEscenaObjeto* objeto){
	SI32 i, iButtonFound = -1; const SI32 count = _options.conteo;
	for(i = 0; i  < count; i++){
		STIMenuOption* button = _options.elemPtr(i);
		if(button->button == objeto){
			button->button->establecerMultiplicadorColor8(100, 100, 100, 255);
			iButtonFound = i;
			break;
		}
	}
}

void AUIMenu::touchMovido(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	//
}

void AUIMenu::touchFinalizado(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	const NBCajaAABB cajaEscena = objeto->cajaAABBEnEscena();
	bool consumed = false;
	//Execute button action
	if(NBCAJAAABB_INTERSECTA_PUNTO(cajaEscena, posActualEscena.x, posActualEscena.y)){
		//Buttons
		SI32 i, iButtonFound = -1; const SI32 count = _options.conteo;
		for(i = 0; i  < count; i++){
			STIMenuOption* button = _options.elemPtr(i);
			if(button->button == objeto){
				if(_listener == NULL){
					button->button->establecerMultiplicadorColor8(255, 0, 0, 255);
					_animObjetos->animarColorMult(button->button, 255, 255, 255, 255, 0.5f);
				} else if(!_listener->menuOptionExecuteAction(this, i)){
					button->button->establecerMultiplicadorColor8(255, 0, 0, 255);
					_animObjetos->animarColorMult(button->button, 255, 255, 255, 255, 0.5f);
				} else {
					button->button->establecerMultiplicadorColor8(0, 255, 0, 255);
					_animObjetos->animarColorMult(button->button, 255, 255, 255, 255, 0.5f);
				}
				iButtonFound = i;
				consumed = true;
				break;
			}
		}
	}
	//Restore button color
	if(!consumed){
		SI32 i, iButtonFound = -1; const SI32 count = _options.conteo;
		for(i = 0; i  < count; i++){
			STIMenuOption* button = _options.elemPtr(i);
			if(button->button == objeto){
				if(!_animObjetos->animandoObjeto(button->button)){
					button->button->establecerMultiplicadorColor8(255, 255, 255, 255);
				}
				iButtonFound = i;
				break;
			}
		}
	}
}

//HOVER
void AUIMenu::hoverIniciado(AUEscenaObjeto* objeto){
	//Buttons
	SI32 i, iButtonFound = -1; const SI32 count = _options.conteo;
	for(i = 0; i  < count; i++){
		STIMenuOption* button = _options.elemPtr(i);
		if(button->button == objeto){
			button->button->establecerMultiplicadorColor8(200, 200, 200, 255);
			iButtonFound = i;
			break;
		}
	}
}

void AUIMenu::hoverMovido(const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	//
}

void AUIMenu::hoverFinalizado(AUEscenaObjeto* objeto){
	//Buttons
	SI32 i, iButtonFound = -1; const SI32 count = _options.conteo;
	for(i = 0; i  < count; i++){
		STIMenuOption* button = _options.elemPtr(i);
		if(button->button == objeto){
			if(!_animObjetos->animandoObjeto(button->button)){
				button->button->establecerMultiplicadorColor8(255, 255, 255, 255);
			}
			iButtonFound = i;
			break;
		}
	}
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUIMenu)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUIMenu, "AUIMenu")
AUOBJMETODOS_CLONAR_NULL(AUIMenu)

