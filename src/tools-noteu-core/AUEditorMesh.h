//
//  AUGpMenuFinPartida.h
//  game-Refranero
//
//  Created by Marcos Ortega on 14/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#ifndef AUEditorMesh_h
#define AUEditorMesh_h

#include "AUIMenu.h"

typedef enum ENEditorMeshRule_ {
	ENEditorMeshRule_Left = 0
	, ENEditorMeshRule_Right
	, ENEditorMeshRule_Top
	, ENEditorMeshRule_Bottom
	, ENEditorMeshRule_Count
} ENEditorMeshRule;

typedef enum ENEditorMeshGrid_ {
	ENEditorMeshGrid_CutH = 0
	, ENEditorMeshGrid_CutV
	, ENEditorMeshGrid_Count
} ENEditorMeshGrid;

#define RANGE_MASK_ITEM_CENTER	1
#define RANGE_MASK_ITEM_MIN		2
#define RANGE_MASK_ITEM_MAX		4

typedef struct STEditorMeshRange_ {
	AUEscenaFigura*	visualRange;	//visual representation
	AUEscenaFigura*	visualMin;		//visual representation
	AUEscenaFigura*	visualMax;		//visual representation
} STEditorMeshRange;

typedef enum ENEditorMeshButton_ {
	ENEditorMeshButton_Close = 0
	, ENEditorMeshButton_Save
	, ENEditorMeshButton_Count
} ENEditorMeshButton;

typedef enum ENEditorMeshTouchType_ {
	ENEditorMeshTouchType_Undef = 0
	, ENEditorMeshTouchType_Drag
} ENEditorMeshTouchType;

//Listener

class AUEditorMesh;

class IListenerEditorMesh {
public:
	virtual ~IListenerEditorMesh(){
		//
	}
	//
	virtual bool editorMeshCloseCommand(AUEditorMesh* obj) = 0;
	virtual bool editorMeshSaveCommand(AUEditorMesh* obj) = 0;
};

class AUEditorMesh: public AUAppEscena, public NBAnimador, public IEscuchadorCambioPuertoVision, public IEscuchadorTouchEscenaObjeto, public ITextEditorListener, public IListenerIMenu {
	public:
		AUEditorMesh(const SI32 iEscena);
		virtual		~AUEditorMesh();
		//
		bool		escenaEnPrimerPlano();
		void		escenaColocarEnPrimerPlano();
		void		escenaQuitarDePrimerPlano();
		bool		escenaPermitidoGirarPantalla();
		bool		escenaEstaAnimandoSalida();
		void		escenaAnimarEntrada();
		void		escenaAnimarSalida();
		void		escenaGetOrientations(UI32* dstMask, ENAppOrientationBit* dstPrefered, ENAppOrientationBit* dstToApplyOnce, BOOL* dstAllowAutoRotate);
		//
		void		appFocoExclusivoObtenido();
		void		appFocoExclusivoPerdido();
		void		appProcesarNotificacionLocal(const char* grp, const SI32 localId, const char* objTip, const SI32 objId);
		//Listener
		IListenerEditorMesh* listener();
		void		setListener(IListenerEditorMesh* listener);
		//
		AUMapaBits*	meshBitmap();
		AUMallaMapa* meshMap();
		//FILES
		bool		openFile(const char* path);
		bool		openBitmap(AUMapaBits* bitmap, AUMallaMapa* meshMap);
		bool		saveMeshBackupFile(AUMallaMapa* meshMap, const char* origFilePath);
		//
		bool		menuOptionExecuteAction(AUIMenu* obj, const SI32 optionId);
		//
		void		puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after);
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
		//TECLAS (old)
		bool		teclaPresionada(SI32 codigoTecla);
		bool		teclaLevantada(SI32 codigoTecla);
		bool		teclaEspecialPresionada(SI32 codigoTecla);
		//TECLAS
		void		textEditorCloneInterfaceObjectsRetained(AUObjeto** dstVisualObj, ITextInputListener** dstVisualItf, AUObjeto** dstValueObj, ITextInputListener** dstValueItf);
		STTextEditorSyncResult textEditorCompareClones(AUObjeto* visualClone, AUObjeto* valueClone);
		STTextEditorSyncResult textEditorSyncClones(AUObjeto* visualClone, AUObjeto* valueClone);
		void		textEditorFocusGained();
		void		textEditorFocusLost(const ENNBKeybFocusLostType actionType);
		bool		textEditorMantainBeforeTouchConsumer(AUEscenaObjeto* touchConsumer);
		//
		void		teclaFocoObtenido();
		void		teclaFocoPerdido();
		bool		teclaMantenerFocoAnteConsumidorDeTouch(AUEscenaObjeto* consumidorTouch);
		bool		teclaTieneTexto();
		void		teclaBackspace();
		void		teclaIntroducirTexto(const char* textoIntroducir);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	protected:
		SI32				_iEscena;
		IListenerEditorMesh* _listener;
		AUAnimadorObjetoEscena* _animObjetos;
		AUEscenaContenedor*	_layerMain;
		AUEscenaContenedor*	_layerEditor;
		AUEscenaContenedor*	_layerPreview;
		AUEscenaContenedor*	_layerDrag;
		AUEscenaContenedor*	_layerGUI;
		//
		AUEscenaTextoEditableOffscreen* _textEditor;
		//
		bool				_enPrimerPlano;
		float				_workspaceScale;
		NBPunto				_workspaceCenter;
		//Editing
		SI32				_meshCoordMutliplier;
		AUCadena8*			_meshFilePath;
		AUMapaBits*			_meshBitmap;
		AUPngChuncksMutable* _meshBitmapExtraChuncks;
		AUTextura*			_meshTexture;
		AUMallaMapaMutable*	_meshMap;
		AUEscenaSprite*		_meshSprite;
		AUEscenaContenedor*	_meshSpriteLayer;
		AUEscenaContenedor*	_meshRulesLayer;	//Texture external rule representation
		AUEscenaContenedor*	_meshGridLayer;		//Texture internal rule representation
		AUArregloMutable*	_meshRules[ENEditorMeshRule_Count]; //AUEscenaFigura
		AUArregloMutable*	_meshGrid[ENEditorMeshGrid_Count]; //AUEscenaFigura
		//GUI
		AUIMenu*			_guiMenu;
		//Ranges
		AUArregloNativoMutableP<STEditorMeshRange>* _meshRanges[ENEditorMeshGrid_Count];
		AUEscenaContenedor*	_meshRangesLayer;
		//Selection
		ENEditorMeshGrid	_selMeshRangeOrientation;
		SI32				_selMeshRangeIndex;
		UI32				_selMeshRangeHoverMask;
		//Preview
		AUEscenaSpriteElastico* _previewSprite;
		AUEscenaSprite*		_previewSpriteDbg;
		bool				_previewGrowingW;
		bool				_previewGrowingH;
		//
		void				privOrganizarContenido();
		void				privWorkspaceSetScale(const float workspaceScale);
		void				privUpdateRangesRepresentations(const bool updateShapes);
		bool				privMeshOpen(const char* imageFilePath);
		bool				privMeshLoadFromBitmap(AUMapaBits* bitmap, AUMallaMapa* meshMap);
		void				privMeshClose();
		bool				privMeshSaveAction();
		bool				privMeshSaveFile();
		void				privInitGrid(const SI32 ancho, const SI32 alto, const float marginFromMesh);
		void				privSelectionSet(const ENEditorMeshGrid orientation, const SI32 index);
		//
		SI32				privIndexOfRangeWithVisualObject(AUEscenaObjeto* obj, const ENEditorMeshGrid rangeOrientation);
		SI32				privRemoveEmptyRanges();
		bool				privRemoveRangeAtIndex(const ENEditorMeshGrid orientation, const SI32 index, const bool addSizeToNext);
};

#endif
