//
//  AUGpMenuFinPartida.h
//  game-Refranero
//
//  Created by Marcos Ortega on 14/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#ifndef AUIMenu_h
#define AUIMenu_h

class AUIMenu;

class IListenerIMenu {
	public:
		virtual ~IListenerIMenu(){
			//
		}
		//
		virtual bool menuOptionExecuteAction(AUIMenu* obj, const SI32 optionId) = 0;
};

typedef struct STIMenuOption_ {
	AUEscenaTexto*	button;
	AUEscenaTexto*	separator;
} STIMenuOption;

class AUIMenu: public AUEscenaContenedor, public NBAnimador, public IEscuchadorTouchEscenaObjeto {
	public:
		AUIMenu();
		virtual		~AUIMenu();
		//
		NBTamano	limitsSize() const;
		void		setLimitsSize(const float width, const float height);
		//
		IListenerIMenu* listener() const ;
		void		setListener(IListenerIMenu* listener);
		//
		void		optionAdd(const char* text);
		void		optionAnimate(const SI32 optionId, const bool animateSuccess);
		void		optionUpdate(const SI32 optionId, const char* text);
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
		IListenerIMenu*	_listener;
		//
		AUFuenteRender*							_optionsFont;
		AUArregloNativoMutableP<STIMenuOption>	_options;
		//
		void		privOrganize();
};

#endif
