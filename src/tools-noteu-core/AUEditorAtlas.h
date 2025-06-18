//
//  AUGpMenuFinPartida.h
//  game-Refranero
//
//  Created by Marcos Ortega on 14/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#ifndef AUEditorAtlas_h
#define AUEditorAtlas_h

#include "AUIListImages.h"
#include "AUIMenu.h"
#include "AUEditorMesh.h"

typedef enum ENEditorAtlasRule_ {
	ENEditorAtlasRule_Left = 0
	, ENEditorAtlasRule_Right
	, ENEditorAtlasRule_Top
	, ENEditorAtlasRule_Bottom
	, ENEditorAtlasRule_Count
} ENEditorAtlasRule;

typedef enum ENEditorAtlasGrid_ {
	ENEditorAtlasGrid_CutH = 0
	, ENEditorAtlasGrid_CutV
	, ENEditorAtlasGrid_Count
} ENEditorAtlasGrid;

typedef enum ENEditorAtlasDragH_ {
	ENEditorAtlasDragH_Left = 0
	, ENEditorAtlasDragH_Right
	, ENEditorAtlasDragH_Undef
} ENEditorAtlasDragH;

typedef enum ENEditorAtlasDragV_ {
	ENEditorAtlasDragV_Up = 0
	, ENEditorAtlasDragV_Down
	, ENEditorAtlasDragV_Undef
} ENEditorAtlasDragV;

typedef struct STEditorAtlasArea_ {
	//Limits
	AUEscenaFigura*		visualArea;	//box, visual representation
	AUEscenaFigura*		visualLimit[ENEditorAtlasRule_Count];	//lines, visual representation
	SI8					visualLimitHoverCount[ENEditorAtlasRule_Count]; //lines have a hover on?
	//Selection bitmap backup
	NBRectanguloUI16	bitMapArea;
	AUMapaBits*			bitMap;
} STEditorAtlasArea;

typedef struct STEditorAtlasAreaMov_ {
	SI32				iArea;				//Area in editor
	AUMapaBits*			copyBitmap;			//Bitmap of copy
	AUTextura*			copyTexture;		//Texture of copy
	AUEscenaSprite*		copySprite;			//Sprite of copy
	NBRectanguloUI16	originalBitMapArea;	//Original area in bitmap
} STEditorAtlasAreaMov;

typedef enum ENEditorAtlasButton_ {
	ENEditorAtlasButton_Save = 0
	, ENEditorAtlasButton_ExportAll
	, ENEditorAtlasButton_Refresh
	, ENEditorAtlasButton_EditMesh
	, ENEditorAtlasButton_PerfectPixelPrefered
	, ENEditorAtlasButton_MaskPrefered
	, ENEditorAtlasButton_TextureMode
	//, ENEditorAtlasButton_InvertColor
	//, ENEditorAtlasButton_FixMasksBorders
	, ENEditorAtlasButton_Count
} ENEditorAtlasButton;

typedef enum ENEditorAtlasTextueMode_ {
	ENEditorAtlasTextueMode_Normal = 0
	, ENEditorAtlasTextueMode_ColorOnly
	, ENEditorAtlasTextueMode_AlphaOnly
	, ENEditorAtlasTextueMode_RedOnly
	, ENEditorAtlasTextueMode_GreenOnly
	, ENEditorAtlasTextueMode_BlueOnly
	, ENEditorAtlasTextueMode_Gray
	, ENEditorAtlasTextueMode_Count
} ENEditorAtlasTextueMode;

// Note: by default, the update method allways
// set the color of the references.
#define EDITOR_ATLAS_UPDATE_MASK_SHAPES					1	//Update refences coordinates and location
#define EDITOR_ATLAS_UPDATE_MASK_BITMAPS_IF_NECESARY	2	//Update the bitmaps backups
#define EDITOR_ATLAS_UPDATE_MASK_BITMAPS_FORCED			4	//Update the bitmaps backups

class AUEditorAtlas: public AUAppEscena, public NBAnimador, public IEscuchadorCambioPuertoVision, public IEscuchadorTouchEscenaObjeto, public ITextEditorListener, public IListenerIListImages, public IListenerIMenu, public IListenerEditorMesh {
	public:
		AUEditorAtlas(SI32 iEscena);
		virtual		~AUEditorAtlas();
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
		//
		void		puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after);
		//
		void		listImagesSelectionChanged(AUIListImages* obj, AUArregloNativoP<SI32>* selectionIndexes);
		void		listImagesNameChaged(AUIListImages* obj, const SI32 imageIndex, const char* newName);
		//
		bool		menuOptionExecuteAction(AUIMenu* obj, const SI32 optionId);
		//
		bool		editorMeshCloseCommand(AUEditorMesh* obj);
		bool		editorMeshSaveCommand(AUEditorMesh* obj);
		//
		void		textboxFocoObtenido(AUITextBox* obj);
		void		textboxFocoPerdido(AUITextBox* obj, const ENNBKeybFocusLostType actionType);
		bool		textboxMantenerFocoAnteConsumidorDeTouch(AUEscenaObjeto* consumidorTouch);
		void		textboxContenidoModificado(AUITextBox* obj, const char* strContenido);
		//
		void		tickAnimacion(float segsTranscurridos);
		//FILES
		bool		openFiles(AUArreglo* paths);
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
		AUAnimadorObjetoEscena* _animObjetos;
		AUEscenaContenedor*	_layerMain;
		AUEscenaContenedor*	_layerDrag;
		AUEscenaContenedor*	_layerGUI;
		//
		AUEscenaTextoEditableOffscreen*		_textEditor;
		//
		bool				_enPrimerPlano;
		float				_workspaceScale;
		NBPunto				_workspaceCenter;
		//Editing
		SI32				_atlasCoordMutliplier;
		AUCadena8*			_atlasFilePath;
		AUMapaBitsLienzo*	_atlasMapaBits;
		AUMapaBits*			_atlasMapaBitsCustomView;
		AUPngChuncksMutable* _atlasMapaBitsExtraChuncks;
		AUTextura*			_atlasTexture;
		ENEditorAtlasTextueMode _atlasTextureMode;
		AUMapaBitsPropsMutable*	_atlasProps;
		AUAtlasMapaMutable*	_atlasMap;
		AUEscenaSprite*		_atlasSprite;
		AUEscenaContenedor*	_atlasSpriteLayer;
		AUEscenaContenedor*	_atlasRulesLayer;	//Texture external rule representation
		AUEscenaContenedor*	_atlasGridLayer;	//Texture internal rule representation
		AUEscenaContenedor*	_atlasAreasLayer;	//
		AUEscenaContenedor*	_atlasAreasSubLayer;//
		AUArregloMutable*	_atlasRules[ENEditorAtlasRule_Count]; //AUEscenaFigura
		AUArregloMutable*	_atlasGrid[ENEditorAtlasGrid_Count]; //AUEscenaFigura
		AUArregloNativoMutableP<STEditorAtlasArea>* _atlasAreas; //STAtlasArea
		//GUI
		AUEditorMesh*		_guiEditorMesh;
		AUIListImages*		_guiImagesList;
		AUIMenu*			_guiMenu;
		//Pixel color
		NBPunto				_colorPxPos;
		NBColor8			_colorValue;
		AUEscenaTexto*		_colorTxt;
		AUEscenaFigura*		_colorBorder;
		AUEscenaFigura*		_colorSample;
		//Area selection
		SI32				_selAreaIndex;
		STGTouch*			_selAreaTouch;
		bool				_selAreaMoving;
		//Area creation
		ENEditorAtlasDragH	_newAreaDragH;
		ENEditorAtlasDragV	_newAreaDragV;
		STGTouch*			_newAreaTouch;	//The current touch event creating a new area
		//Areas movement
		AUArregloNativoMutableP<STEditorAtlasAreaMov> _movAreaCopies;
		float				_movDeltaXAcum;	//For acurate movement
		float				_movDeltaYAcum;	//For acurate movement
		bool				_movColisionDetected;
		//Atlas data
		void				privOrganizarContenido();
		void				privWorkspaceSetScale(const float workspaceScale);
		SI32				privIndexAreaWithObject(AUEscenaObjeto* obj);
		SI32				privRemoveEmptyAreas();
		void				privUpdateAreasRepresentations(const SI32 updateMask);
		//
		bool				privAtlasNew(const SI32 width, const SI32 height);
		void				privInitRulesAndGrid(const SI32 width, const SI32 height, const SI32 marginFromMesh);
		void				privAtlasClose();
		bool				privAtlasSave();
		bool				privAtlasExportAll();
		void				privAtlasRefreshTexture();
		void				privTextureSwitchShowColor();
		//Areas movement
		void				privMovFill();
		void				privMovClear(const bool applyMovements);
		void				privMovApplyDelta(const float deltaX, const float deltaY);
		bool				privMovAreaColides();
};

#endif
