//
//  AUIListImages.cpp
//  app-refranero
//
//  Created by Marcos Ortega on 14/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#include "NoteuCorePrecomp.h"
#include "AUIListImages.h"
//

#ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
#	define AUILISTIMAGES_VERIFY_SELECTION	NBASSERT(this->privDebgSelectionVerify())
#else
#	define AUILISTIMAGES_VERIFY_SELECTION
#endif
	
AUIListImages::AUIListImages() : AUEscenaContenedor()
	, _animObjetos(NULL)
    , _list(this)
    , _listSelectionIndexes(this)
    , _listener(NULL)
    , _layer(NULL)
    , _background(NULL)
{
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUIListImages")
	//
	_animObjetos		= new(this) AUAnimadorObjetoEscena();
	_limitsSize.ancho	= _limitsSize.alto = 0.0f;
	//
	NBCOLOR_ESTABLECER(bgColorNormal, 255, 255, 255, 255)
	//NBCOLOR_ESTABLECER(bgColorNormal, 255, 191, 37, 255)
	NBCOLOR_ESTABLECER(txtColorNormal, 0, 0, 0, 255)
	NBCOLOR_ESTABLECER(extrasColorNormal, 100, 100, 100, 255)
	//
	NBCOLOR_ESTABLECER(bgColorSelected, 100, 100, 255, 255)
	NBCOLOR_ESTABLECER(txtColorSelected, 255, 255, 255, 255)
	NBCOLOR_ESTABLECER(extrasColorSelected, 155, 155, 155, 255)
	//
	_background			= new(this) AUEscenaSprite();
	_background->establecerMultiplicadorColor8(101, 101, 101, 255);
	_background->establecerEscuchadorTouches(this, this);
	this->agregarObjetoEscena(_background);
	//
	_layer				= new(this) AUEscenaContenedor();
	this->agregarObjetoEscena(_layer);
	//
	NBGestorAnimadores::agregarAnimador(this, this);
}

AUIListImages::~AUIListImages(){
	NBGestorAnimadores::quitarAnimador(this);
	//
	if(_animObjetos!=NULL) _animObjetos->liberar(NB_RETENEDOR_THIS); _animObjetos = NULL;
	//
	if(_layer != NULL) _layer->liberar(); _layer = NULL;
	if(_background != NULL) _background->liberar(NB_RETENEDOR_THIS); _background = NULL;
	//
	this->privListClear();
}

//

NBTamano AUIListImages::limitsSize() const {
	return _limitsSize;
}

void AUIListImages::setLimitsSize(const float width, const float height){
	_limitsSize.ancho	= width;
	_limitsSize.alto	= height;
	this->privOrganize();
}

//

IListenerIListImages* AUIListImages::listener() const {
	return _listener;
}

void AUIListImages::setListener(IListenerIListImages* listener){
	_listener = listener;
}

//

void AUIListImages::clear(){
	this->privListClear();
	this->privOrganize();
}

SI32 AUIListImages::imageCount() const {
	return (SI32)_list.conteo;
}

void AUIListImages::addImage(AUMapaBits* image, const char* name, const UI8 scaleBase2, const float scaleForHD){
	this->privListAddImageAtIndex(_list.conteo, image, name, scaleBase2, scaleForHD);
	this->privOrganize();
}

void AUIListImages::addImageAtIndex(const SI32 index, AUMapaBits* image, const char* name, const UI8 scaleBase2, const float scaleForHD){
	this->privListAddImageAtIndex(index, image, name, scaleBase2, scaleForHD);
	this->privOrganize();
}

void AUIListImages::updateImageAtIndex(const SI32 index, AUMapaBits* image, const UI8 scaleBase2, const float scaleForHD){
	this->privListUpdateImageAtIndex(index, image, scaleBase2, scaleForHD);
	this->privOrganize();
}

void AUIListImages::updateImageNameAtIndex(const SI32 index, const char* name){
	this->privListUpdateImageNameAtIndex(index, name);
}

void AUIListImages::updateImageExtraDetailsAtIndex(const SI32 index, const char* extraDetails){
	this->privUpdateImageExtraDetailsAtIndex(index, extraDetails);
}

void AUIListImages::removeImageAtIndex(const SI32 index){
	this->privListRemoveImageAtIndex(index, true);
	this->privOrganize();
}

// Selection

void AUIListImages::selectionClear(){
	//Update presentation
	SI32 i;
	for(i = 0; i < _listSelectionIndexes.conteo; i++){
		const SI32* index = _listSelectionIndexes.elemPtr(i);
		STIListImagesImage* img = _list.elemPtr(*index);
		img->background->establecerMultiplicadorColor8(bgColorNormal.r, bgColorNormal.g, bgColorNormal.b, img->background->_propiedades.color8.a);
		img->txtName->establecerMultiplicadorColor8(txtColorNormal.r, txtColorNormal.g, txtColorNormal.b, img->txtName->_propiedades.color8.a);
		if(img->txtExtraDetails != NULL){
			img->txtExtraDetails->establecerMultiplicadorColor8(extrasColorNormal.r, extrasColorNormal.g, extrasColorNormal.b, img->txtExtraDetails->_propiedades.color8.a);
		}
	}
	_listSelectionIndexes.vaciar();
	if(_listener != NULL){ _listener->listImagesSelectionChanged(this, &_listSelectionIndexes); }
	AUILISTIMAGES_VERIFY_SELECTION
}

void AUIListImages::selectionSetIndex(const SI32 index){
	if(index >= 0 && index < _list.conteo){
		//Clear
		SI32 i;
		for(i = 0; i < _listSelectionIndexes.conteo; i++){
			const SI32* index = _listSelectionIndexes.elemPtr(i);
			STIListImagesImage* img = _list.elemPtr(*index);
			img->background->establecerMultiplicadorColor8(bgColorNormal.r, bgColorNormal.g, bgColorNormal.b, img->background->_propiedades.color8.a);
			img->txtName->establecerMultiplicadorColor8(txtColorNormal.r, txtColorNormal.g, txtColorNormal.b, img->txtName->_propiedades.color8.a);
			if(img->txtExtraDetails != NULL){
				img->txtExtraDetails->establecerMultiplicadorColor8(extrasColorNormal.r, extrasColorNormal.g, extrasColorNormal.b, img->txtExtraDetails->_propiedades.color8.a);
			}
		}
		_listSelectionIndexes.vaciar();
		//Add
		_listSelectionIndexes.agregarElemento(index);
		STIListImagesImage* img = _list.elemPtr(index);
		img->background->establecerMultiplicadorColor8(bgColorSelected.r, bgColorSelected.g, bgColorSelected.b, img->background->_propiedades.color8.a);
		img->txtName->establecerMultiplicadorColor8(txtColorSelected.r, txtColorSelected.g, txtColorSelected.b, img->txtName->_propiedades.color8.a);
		if(img->txtExtraDetails != NULL){
			img->txtExtraDetails->establecerMultiplicadorColor8(extrasColorSelected.r, extrasColorSelected.g, extrasColorSelected.b, img->txtExtraDetails->_propiedades.color8.a);
		}
		//
		if(_listener != NULL){ _listener->listImagesSelectionChanged(this, &_listSelectionIndexes); }
	}
	AUILISTIMAGES_VERIFY_SELECTION
}

void AUIListImages::selectionAddIndex(const SI32 index){
	if(index >= 0 && index < _list.conteo){
		if(_listSelectionIndexes.indiceDe(index) == -1){
			_listSelectionIndexes.agregarElemento(index);
			//Update presentation
			STIListImagesImage* img = _list.elemPtr(index);
			img->background->establecerMultiplicadorColor8(bgColorSelected.r, bgColorSelected.g, bgColorSelected.b, img->background->_propiedades.color8.a);
			img->txtName->establecerMultiplicadorColor8(txtColorSelected.r, txtColorSelected.g, txtColorSelected.b, img->txtName->_propiedades.color8.a);
			if(img->txtExtraDetails != NULL){
				img->txtExtraDetails->establecerMultiplicadorColor8(extrasColorSelected.r, extrasColorSelected.g, extrasColorSelected.b, img->txtExtraDetails->_propiedades.color8.a);
			}
		}
	}
	if(_listener != NULL){ _listener->listImagesSelectionChanged(this, &_listSelectionIndexes); }
	AUILISTIMAGES_VERIFY_SELECTION
}

void AUIListImages::selectionRemoveIndex(const SI32 index){
	if(index >= 0 && index < _list.conteo){
		const SI32 ii = _listSelectionIndexes.indiceDe(index);
		if(ii != -1){
			_listSelectionIndexes.quitarElementoEnIndice(ii);
			//Update presentation
			STIListImagesImage* img = _list.elemPtr(index);
			img->background->establecerMultiplicadorColor8(bgColorNormal.r, bgColorNormal.g, bgColorNormal.b, img->background->_propiedades.color8.a);
			img->txtName->establecerMultiplicadorColor8(txtColorNormal.r, txtColorNormal.g, txtColorNormal.b, img->txtName->_propiedades.color8.a);
			if(img->txtExtraDetails != NULL){
				img->txtExtraDetails->establecerMultiplicadorColor8(extrasColorNormal.r, extrasColorNormal.g, extrasColorNormal.b, img->txtExtraDetails->_propiedades.color8.a);
			}
		}
	}
	if(_listener != NULL){ _listener->listImagesSelectionChanged(this, &_listSelectionIndexes); }
	AUILISTIMAGES_VERIFY_SELECTION
}

bool AUIListImages::selectionHaveIndex(const SI32 index){
	bool r = (_listSelectionIndexes.indiceDe(index) != -1);
	return r;
}

const AUArregloNativoP<SI32>* AUIListImages::selectionIndexes() const {
	return &_listSelectionIndexes;
}

#ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
bool AUIListImages::privDebgSelectionVerify(){
	bool r = true;
	SI32 i; const SI32 conteo = _list.conteo;
	for(i = 0; i < conteo; i++){
		const bool isSelected	= (_listSelectionIndexes.indiceDe(i) >= 0);
		STIListImagesImage* img	= _list.elemPtr(i);
		const NBColor8 colorBg	= img->background->_propiedades.color8;
		const NBColor8 colorTxtName	= img->txtName->_propiedades.color8;
		if(isSelected){
			NBASSERT(colorBg.r == bgColorSelected.r && colorBg.g == bgColorSelected.g && colorBg.b == bgColorSelected.b)
			NBASSERT(colorTxtName.r == txtColorSelected.r && colorTxtName.g == txtColorSelected.g && colorTxtName.b == txtColorSelected.b)
		} else {
			NBASSERT(colorBg.r == bgColorNormal.r && colorBg.g == bgColorNormal.g && colorBg.b == bgColorNormal.b)
			NBASSERT(colorTxtName.r == txtColorNormal.r && colorTxtName.g == txtColorNormal.g && colorTxtName.b == txtColorNormal.b)
		}
		if(img->txtExtraDetails != NULL){
			const NBColor8 colorTxtDet = img->txtExtraDetails->_propiedades.color8;
			if(isSelected){
				NBASSERT(img->txtExtraDetails == NULL || (colorTxtDet.r == extrasColorSelected.r && colorTxtDet.g == extrasColorSelected.g && colorTxtDet.b == extrasColorSelected.b))
			} else {
				NBASSERT(img->txtExtraDetails == NULL || (colorTxtDet.r == extrasColorNormal.r && colorTxtDet.g == extrasColorNormal.g && colorTxtDet.b == extrasColorNormal.b))
			}
		}
	}
	return r;
}
#endif

// TextBoxs

void AUIListImages::textboxFocoObtenido(AUITextBox* obj){
	SI32 i; const SI32 count = _list.conteo;
	for(i = 0; i < count; i++){
		STIListImagesImage* img = _list.elemPtr(i);
		if(img->inputName == obj){
			img->txtName->establecerVisible(false);
			img->inputName->establecerVisible(true);
			break;
		}
	}
}

void AUIListImages::textboxFocoPerdido(AUITextBox* obj, const ENNBKeybFocusLostType actionType){
	SI32 i; const SI32 count = _list.conteo;
	for(i = 0; i < count; i++){
		STIListImagesImage* img = _list.elemPtr(i);
		if(img->inputName == obj){
			img->txtName->establecerVisible(true);
			img->inputName->establecerVisible(false);
			break;
		}
	}
}

bool AUIListImages::textboxMantenerFocoAnteConsumidorDeTouch(AUITextBox* obj, AUEscenaObjeto* consumidorTouch){
	return false;
}

void AUIListImages::textboxContenidoModificado(AUITextBox* obj, const char* strContenido){
	SI32 i; const SI32 count = _list.conteo;
	for(i = 0; i < count; i++){
		STIListImagesImage* img = _list.elemPtr(i);
		if(img->inputName == obj){
			if(strContenido == NULL){
				img->txtName->establecerTexto("[unnamed]");
			} else if(strContenido[0] == '\0'){
				img->txtName->establecerTexto("[unnamed]");
			} else {
				img->txtName->establecerTexto(strContenido);
			}
			if(_listener != NULL){
				_listener->listImagesNameChaged(this, i, strContenido);
			}
			break;
		}
	}
}

//

void AUIListImages::privListClear(){
	const bool notifyListener = (_listSelectionIndexes.conteo != 0);
	SI32 i; const SI32 count = _list.conteo;
	for(i = count - 1; i >= 0; i--){
		this->privListRemoveImageAtIndex(i, false);
	}
	if(notifyListener && _listener != NULL){ _listener->listImagesSelectionChanged(this, &_listSelectionIndexes); }
	AUILISTIMAGES_VERIFY_SELECTION
}

void AUIListImages::privListRemoveImageAtIndex(const SI32 iImage, const bool notifyListener){
	NBASSERT(iImage >= 0 && iImage < _list.conteo)
	if(iImage >= 0 && iImage < _list.conteo){
		STIListImagesImage* img = _list.elemPtr(iImage);
		if(img->background != NULL) img->background->liberar();
		if(img->image != NULL) img->image->liberar();
		if(img->txtExtraDetails != NULL) img->txtExtraDetails->liberar();
		if(img->txtName != NULL) img->txtName->liberar();
		if(img->inputName != NULL) img->inputName->liberar();
		if(img->container != NULL){
			AUEscenaContenedor* parent = (AUEscenaContenedor*)img->container->contenedor();
			if(parent != NULL) { parent->quitarObjetoEscena(img->container); }
			img->container->liberar();
			img->container = NULL;
		}
		_list.quitarElementoEnIndice(iImage);
		//Remove from selection
		SI32 i, countSelIndexesUpdated = 0;
		for(i = (_listSelectionIndexes.conteo - 1); i >= 0 ; i--){
			SI32* index = _listSelectionIndexes.elemPtr(i);
			if(*index == iImage){
				//Remove
				_listSelectionIndexes.quitarElementoEnIndice(i);
				countSelIndexesUpdated++;
			} else if(*index > iImage){
				//Update bigger indexes
				*index = *index - 1;
				countSelIndexesUpdated++;
			}
		}
		//
		if(notifyListener && countSelIndexesUpdated != 0 && _listener != NULL){ _listener->listImagesSelectionChanged(this, &_listSelectionIndexes); }
	}
	AUILISTIMAGES_VERIFY_SELECTION
}

void AUIListImages::privListAddImageAtIndex(const SI32 iImage, AUMapaBits* image, const char* name, const UI8 scaleBase2, const float scaleForHD){
	NBASSERT(iImage >= 0 && iImage <= _list.conteo)
	if(iImage >= 0 && iImage <= _list.conteo){
		AUFuenteTextura* fuente = NBGestorTexturas::fuenteTextura("Helvetica", FONT_SIZE_PIX, false, false);
		//
		STIListImagesImage img;
		img.container	= new(this) AUEscenaContenedor();
		img.background	= new(this) AUEscenaSprite();
		img.background->establecerMultiplicadorColor8(bgColorNormal.r, bgColorNormal.g, bgColorNormal.b, 150);
		img.background->establecerEscuchadorTouches(this, this);
		if(image == NULL){
			img.image	= new(this) AUEscenaSprite();
		} else {
			img.image	= new(this) AUEscenaSprite(NBGestorTexturas::texturaDesdeMapaBits(image, "", ENTexturaModoPintado_Imagen_Suavizada, ENTexturaMipMap_Inhabilitado, ENTexturaOrdenV_HaciaArriba, scaleBase2, scaleForHD), ENSpriteAlineaH_Der, ENSpriteAlineaV_Aba);
		}
		img.txtExtraDetails	= NULL;
		img.txtName		= new(this) AUEscenaTexto(fuente);
		if(name == NULL){
			img.txtName->establecerTexto("[unnamed]");
		} else if(name[0] == '\0'){
			img.txtName->establecerTexto("[unnamed]");
		} else {
			img.txtName->establecerTexto(name);
		}
		img.txtName->establecerMultiplicadorColor8(txtColorNormal.r, txtColorNormal.g, txtColorNormal.b, 255);
		img.txtName->establecerEscuchadorTouches(this, this);
		img.inputName	= new(this) AUITextBox(ENTextEditorType_Literal, fuente, 200.0f, 1.0f);
		img.inputName->establecerVisible(false);
		img.inputName->establecerTexto(name);
		img.inputName->establecerEscuchadorTextBox(this);
		img.container->agregarObjetoEscena(img.background);
		img.container->agregarObjetoEscena(img.image);
		img.container->agregarObjetoEscena(img.txtName);
		img.container->agregarObjetoEscena(img.inputName);
		//Update bigger indexes in selection
		SI32 countSelIndexesUpdated = 0;
		if(iImage < _list.conteo){
			SI32 i;
			for(i = (_listSelectionIndexes.conteo - 1); i >= 0 ; i--){
				SI32* index = _listSelectionIndexes.elemPtr(i);
				if(*index >= iImage){
					*index = *index + 1;
					countSelIndexesUpdated++;
				}
			}
		}
		//
		_list.agregarElemento(img);
		_layer->agregarObjetoEscena(img.container);
		//
		if(countSelIndexesUpdated != 0 && _listener != NULL){ _listener->listImagesSelectionChanged(this, &_listSelectionIndexes); }
	}
	AUILISTIMAGES_VERIFY_SELECTION
}

void AUIListImages::privListUpdateImageAtIndex(const SI32 iImage, AUMapaBits* image, const UI8 scaleBase2, const float scaleForHD){
	NBASSERT(iImage >= 0 && iImage < _list.conteo)
	if(iImage >= 0 && iImage < _list.conteo){
		STIListImagesImage* img = _list.elemPtr(iImage);
		if(img->image != NULL){
			AUEscenaContenedor* parent = (AUEscenaContenedor*)img->image->contenedor();
			if(parent != NULL) { parent->quitarObjetoEscena(img->image); }
			img->image->liberar();
			img->image = NULL;
		}
		if(image == NULL){
			img->image	= new(this) AUEscenaSprite();
		} else {
			img->image	= new(this) AUEscenaSprite(NBGestorTexturas::texturaDesdeMapaBits(image, "", ENTexturaModoPintado_Imagen_Suavizada, ENTexturaMipMap_Inhabilitado, ENTexturaOrdenV_HaciaArriba, scaleBase2, scaleForHD), ENSpriteAlineaH_Der, ENSpriteAlineaV_Aba);
		}
		img->container->agregarObjetoEscena(img->image);
	}
	AUILISTIMAGES_VERIFY_SELECTION
}

void AUIListImages::privListUpdateImageNameAtIndex(const SI32 iImage, const char* name){
	NBASSERT(iImage >= 0 && iImage < _list.conteo)
	if(iImage >= 0 && iImage < _list.conteo){
		STIListImagesImage* img = _list.elemPtr(iImage);
		if(name == NULL){
			img->txtName->establecerTexto("[unnamed]");
		} else if(name[0] == '\0'){
			img->txtName->establecerTexto("[unnamed]");
		} else {
			img->txtName->establecerTexto(name);
		}
		img->inputName->establecerTexto(name);
	}
	AUILISTIMAGES_VERIFY_SELECTION
}

void AUIListImages::privUpdateImageExtraDetailsAtIndex(const SI32 iImage, const char* extraDetails){
	NBASSERT(iImage >= 0 && iImage < _list.conteo)
	if(iImage >= 0 && iImage < _list.conteo){
		STIListImagesImage* img = _list.elemPtr(iImage);
		if(extraDetails == NULL){
			if(img->txtExtraDetails != NULL){
				AUEscenaContenedor* parent = (AUEscenaContenedor*)img->txtExtraDetails->contenedor();
				if(parent != NULL) parent->quitarObjetoEscena(img->txtExtraDetails);
				img->txtExtraDetails->liberar();
				img->txtExtraDetails = NULL;
			}
		} else if(extraDetails[0] == '\0'){
			if(img->txtExtraDetails != NULL){
				AUEscenaContenedor* parent = (AUEscenaContenedor*)img->txtExtraDetails->contenedor();
				if(parent != NULL) parent->quitarObjetoEscena(img->txtExtraDetails);
				img->txtExtraDetails->liberar();
				img->txtExtraDetails = NULL;
			}
		} else {
			if(img->txtExtraDetails == NULL){
				img->txtExtraDetails = new(this) AUEscenaTexto(NBGestorTexturas::fuenteTextura("Helvetica", FONT_SIZE_PIX, false, false));
				img->txtExtraDetails->establecerMultiplicadorColor8(extrasColorNormal.r, extrasColorNormal.g, extrasColorNormal.b, 255);
				img->container->agregarObjetoEscena(img->txtExtraDetails);
			}
			img->txtExtraDetails->establecerTexto(extraDetails);
		}
	}
	AUILISTIMAGES_VERIFY_SELECTION
}

SI32 AUIListImages::privListGetImageIndexWithObject(AUEscenaObjeto* obj){
	SI32 r = -1;
	SI32 i; const SI32 count = _list.conteo;
	for(i = 0; i < count; i++){
		STIListImagesImage* img = _list.elemPtr(i);
		if(obj == img->background || obj == img->container || obj == img->image || obj == img->txtName || obj == img->txtExtraDetails || obj == img->inputName){
			r = i;
			break;
		}
	}
	return r;
}

//

void AUIListImages::privOrganize(){
	if(_limitsSize.ancho <= 0.0f || _limitsSize.alto <= 0.0f){
		//Hide all elements
		_layer->establecerVisible(false);
		_background->establecerVisible(false);
	} else {
		const float marginLeft		= 2.0f;
		const float marginRight		= 2.0f;
		const float marginInternal	= 4.0f;
		const float separationMargin = 4.0f;
		//Vertical
		const float widthForImage = (_limitsSize.ancho - marginLeft - marginRight - marginInternal - marginInternal);
		float ySup = 0.0f;
		SI32 i; const SI32 count = _list.conteo;
		for(i = 0; i< count; i++){
			NBTamano imgSize;
			STIListImagesImage* img	= _list.elemPtr(i);
			AUTextura* imgTex		= img->image->textura();
			if(imgTex == NULL){
				imgSize.ancho = imgSize.alto = 25.0f;
			} else {
				imgSize	= imgTex->tamanoHD();
			}
			float scaleToUse = widthForImage / (imgSize.alto > imgSize.ancho ? imgSize.alto : imgSize.ancho);
			if(scaleToUse > 1.0f) scaleToUse = 1.0f;
			NBTamano sizeScaled;
			sizeScaled.ancho = (imgSize.ancho * scaleToUse);
			sizeScaled.alto  = (imgSize.alto  * scaleToUse);
			//
			ySup -= (separationMargin * 0.5f);
			const float backgroundYSup = ySup;
			img->background->establecerTraslacion(marginLeft, ySup);
			//Image
			{
				ySup -= marginInternal;
				img->image->establecerTraslacion(marginLeft + marginInternal + ((widthForImage - sizeScaled.ancho) * 0.5f), ySup);
				img->image->establecerEscalacion(scaleToUse);
				ySup -= sizeScaled.alto;
			}
			//Name
			const float nameYSup = ySup;
			float nameTxtYInf = ySup, nameInputYInf = ySup;
			//Name-text
			{
				ySup = nameYSup;
				ySup -= marginInternal;
				const NBCajaAABB detLocalBox = img->txtName->cajaAABBLocalCalculada();
				float wScale = widthForImage / (detLocalBox.xMax - detLocalBox.xMin); if(wScale > 1.0f) wScale = 1.0f;
				img->txtName->establecerEscalacion(wScale);
				img->txtName->establecerTraslacion((widthForImage - ((detLocalBox.xMax - detLocalBox.xMin) * wScale)) * 0.5f, ySup - detLocalBox.yMax);
				ySup -= (detLocalBox.yMax - detLocalBox.yMin) * wScale;
				nameTxtYInf = ySup;
			}
			//Name-input
			{
				ySup = nameYSup;
				ySup -= marginInternal;
				img->inputName->establecerTamanoCaja(widthForImage, 1.0f);
				const NBCajaAABB nameLocalBox = img->inputName->cajaAABBLocalCalculada();
				img->inputName->establecerTraslacion(marginLeft + marginInternal - nameLocalBox.xMin, ySup - nameLocalBox.yMax);
				ySup -= (nameLocalBox.yMax - nameLocalBox.yMin);
				nameInputYInf = ySup;
			}
			ySup = (nameTxtYInf < nameInputYInf ? nameTxtYInf : nameInputYInf);
			//Extra details
			if(img->txtExtraDetails != NULL){
				ySup -= marginInternal;
				const NBCajaAABB detLocalBox = img->txtExtraDetails->cajaAABBLocalCalculada();
				float wScale = widthForImage / (detLocalBox.xMax - detLocalBox.xMin); if(wScale > 1.0f) wScale = 1.0f;
				img->txtExtraDetails->establecerEscalacion(wScale);
				img->txtExtraDetails->establecerTraslacion((widthForImage - ((detLocalBox.xMax - detLocalBox.xMin) * wScale)) * 0.5f, ySup - detLocalBox.yMax);
				ySup -= (detLocalBox.yMax - detLocalBox.yMin) * wScale;
			}
			//
			ySup -= marginInternal;
			img->background->redimensionar(0.0f, 0.0f, (widthForImage + marginInternal + marginInternal), ySup - backgroundYSup);
			ySup -= (separationMargin * 0.5f);
		}
		_layer->establecerVisible(true);
		//Background
		_background->establecerVisible(true);
		_background->redimensionar(0.0f, -_limitsSize.alto, _limitsSize.ancho, _limitsSize.alto);
	}
	AUILISTIMAGES_VERIFY_SELECTION
}

//

void AUIListImages::tickAnimacion(float segsTranscurridos){
	//
}

//TOUCHES
void AUIListImages::touchIniciado(STGTouch* touch, const NBPunto &posTouchEscena, AUEscenaObjeto* objeto){
	//
}

void AUIListImages::touchMovido(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	if(objeto == _background){
		//Vertical scroll
		const NBCajaAABB contentBox = _layer->cajaAABBLocal();
		const NBPunto curPos = _layer->traslacion();
		float yDst = curPos.y + (posActualEscena.y - posAnteriorEscena.y);
		if(yDst > (contentBox.yMax - contentBox.yMin) - _limitsSize.alto){ yDst = (contentBox.yMax - contentBox.yMin) - _limitsSize.alto; }
		if(yDst < 0.0f){ yDst = 0.0f; }
		_layer->establecerTraslacion(curPos.x, yDst);
	} else if(this->privListGetImageIndexWithObject(objeto) != -1){
		const float scrollFromStart = (posActualEscena.y - posInicialEscena.y);
		if(scrollFromStart < -16.0f || scrollFromStart > 16.0f){
			objeto->liberarTouch(touch, posInicialEscena, posAnteriorEscena, posActualEscena, true, _background);
		}
	}
}

void AUIListImages::touchFinalizado(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	const NBCajaAABB cajaEscena = objeto->cajaAABBEnEscena();
	if(NBCAJAAABB_INTERSECTA_PUNTO(cajaEscena, posActualEscena.x, posActualEscena.y)){
		//Click over an image
		if(!touch->cancelado){
			const SI32 iImage = this->privListGetImageIndexWithObject(objeto);
			if(iImage != -1){
				STIListImagesImage* img = _list.elemPtr(iImage);
				if(objeto == img->txtName){
					NBGestorTeclas::escuchadorEstablecer(img->inputName, img->inputName);
				} else {
					if(this->selectionHaveIndex(iImage)){
						this->selectionRemoveIndex(iImage);
					} else {
						this->selectionAddIndex(iImage);
					}
				}
			}
		}
	}
}

//HOVER
void AUIListImages::hoverIniciado(AUEscenaObjeto* objeto){
	const SI32 iImage = this->privListGetImageIndexWithObject(objeto);
	if(iImage != -1){
		STIListImagesImage* img = _list.elemPtr(iImage);
		img->background->establecerMultiplicadorAlpha8(255);
	}
}

void AUIListImages::hoverMovido(const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	//
}

void AUIListImages::hoverFinalizado(AUEscenaObjeto* objeto){
	const SI32 iImage = this->privListGetImageIndexWithObject(objeto);
	if(iImage != -1){
		STIListImagesImage* img = _list.elemPtr(iImage);
		img->background->establecerMultiplicadorAlpha8(150);
	}
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUIListImages)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUIListImages, "AUIListImages")
AUOBJMETODOS_CLONAR_NULL(AUIListImages)

