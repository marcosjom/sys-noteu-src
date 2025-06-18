//
//  AUGpMenuFinPartida.h
//  game-Refranero
//
//  Created by Marcos Ortega on 14/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#ifndef AUIListImages_h
#define AUIListImages_h

class AUIListImages;

class IListenerIListImages {
	public:
		virtual ~IListenerIListImages(){
			//
		}
		//
		virtual void listImagesSelectionChanged(AUIListImages* obj, AUArregloNativoP<SI32>* selectionIndexes) = 0;
		virtual void listImagesNameChaged(AUIListImages* obj, const SI32 imageIndex, const char* newName) = 0;
};

typedef struct STIListImagesImage_ {
	AUEscenaContenedor* container;
	AUEscenaSprite*		background;
	AUEscenaSprite*		image;
	AUEscenaTexto*		txtExtraDetails;
	AUEscenaTexto*		txtName;
	AUITextBox*			inputName;
} STIListImagesImage;

class AUIListImages: public AUEscenaContenedor, public NBAnimador, public IEscuchadorTouchEscenaObjeto, public IEscuchadorITextBox {
	public:
		AUIListImages();
		virtual		~AUIListImages();
		//
		NBTamano	limitsSize() const;
		void		setLimitsSize(const float width, const float height);
		//
		IListenerIListImages* listener() const ;
		void		setListener(IListenerIListImages* listener);
		//
		void		clear();
		SI32		imageCount() const;
		void		addImage(AUMapaBits* image, const char* name, const UI8 scaleBase2, const float scaleForHD);
		void		addImageAtIndex(const SI32 index, AUMapaBits* image, const char* name, const UI8 scaleBase2, const float scaleForHD);
		void		updateImageAtIndex(const SI32 index, AUMapaBits* image, const UI8 scaleBase2, const float scaleForHD);
		void		updateImageNameAtIndex(const SI32 index, const char* name);
		void		updateImageExtraDetailsAtIndex(const SI32 index, const char* extraDetails);
		void		removeImageAtIndex(const SI32 index);
		//
		void		selectionClear();
		void		selectionSetIndex(const SI32 index);
		void		selectionAddIndex(const SI32 index);
		void		selectionRemoveIndex(const SI32 index);
		bool		selectionHaveIndex(const SI32 index);
		const AUArregloNativoP<SI32>* selectionIndexes() const;
		//
		void		textboxFocoObtenido(AUITextBox* obj);
		void		textboxFocoPerdido(AUITextBox* obj, const ENNBKeybFocusLostType actionType);
		bool		textboxMantenerFocoAnteConsumidorDeTouch(AUITextBox* obj, AUEscenaObjeto* consumidorTouch);
		void		textboxContenidoModificado(AUITextBox* obj, const char* strContenido);
		//
		void		tickAnimacion(float segsTranscurridos);
		//TOUCHES
		void		touchIniciado(STGTouch* touch, const NBPunto &posTouchEscena, AUEscenaObjeto* objeto);
		void		touchMovido(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto);
		void		touchFinalizado(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto);
		//HOVERS
		void		hoverIniciado(AUEscenaObjeto* objeto);
		void		hoverMovido(const NBPunto &posActualEscena, AUEscenaObjeto* objeto);
		void		hoverFinalizado(AUEscenaObjeto* objeto);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	protected:
		NBTamano	_limitsSize;
		AUAnimadorObjetoEscena* _animObjetos;
		AUArregloNativoMutableP<STIListImagesImage> _list;
		AUArregloNativoMutableP<SI32> _listSelectionIndexes;
		IListenerIListImages*	_listener;
		//
		NBColor8	bgColorNormal;
		NBColor8	txtColorNormal;
		NBColor8	extrasColorNormal;
		//
		NBColor8	bgColorSelected;
		NBColor8	txtColorSelected;
		NBColor8	extrasColorSelected;
		//
		AUEscenaContenedor* _layer;
		AUEscenaSprite* _background;
		//
		void		privOrganize();
		void		privListClear();
		void		privListRemoveImageAtIndex(const SI32 iImage, const bool notifyListener);
		void		privListAddImageAtIndex(const SI32 iImage, AUMapaBits* image, const char* name, const UI8 scaleBase2, const float scaleForHD);
		void		privListUpdateImageAtIndex(const SI32 iImage, AUMapaBits* image, const UI8 scaleBase2, const float scaleForHD);
		void		privListUpdateImageNameAtIndex(const SI32 iImage, const char* name);
		void		privUpdateImageExtraDetailsAtIndex(const SI32 iImage, const char* extraDetails);
		//
		SI32		privListGetImageIndexWithObject(AUEscenaObjeto* obj);
#		ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
		bool		privDebgSelectionVerify();
#		endif
};

#endif
