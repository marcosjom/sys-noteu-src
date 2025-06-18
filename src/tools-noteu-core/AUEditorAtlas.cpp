//
//  AUEditorAtlas.cpp
//  app-refranero
//
//  Created by Marcos Ortega on 14/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#include "NoteuCorePrecomp.h"
#include "AUEditorAtlas.h"
#include "NBAtlasTools.h"
//

AUEditorAtlas::AUEditorAtlas(SI32 iEscena) : AUAppEscena()
	, _iEscena(iEscena)
	, _animObjetos(NULL)
	, _layerMain(NULL)
	, _layerDrag(NULL)
    , _layerGUI(NULL)
	, _enPrimerPlano(false)
	, _workspaceScale(1.0f)
    , _atlasCoordMutliplier(8)
	, _atlasFilePath(NULL)
	, _atlasMapaBits(NULL)
	, _atlasMapaBitsCustomView(NULL)
    , _atlasMapaBitsExtraChuncks(NULL)
	, _atlasTexture(NULL)
	, _atlasTextureMode(ENEditorAtlasTextueMode_Normal)
	, _atlasProps(NULL)
    , _atlasMap(NULL)
	, _atlasSprite(NULL)
	//
    , _guiEditorMesh(NULL)
    , _guiImagesList(NULL)
    , _guiMenu(NULL)
	//
    , _selAreaIndex(-1)
	, _selAreaTouch(NULL)
    , _newAreaTouch(NULL)
    , _movAreaCopies(this)
    , _movDeltaXAcum(0.0f)
    , _movDeltaYAcum(0.0f)
    , _movColisionDetected(false)
{
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUEditorAtlas")
	//
	_animObjetos	= new(this) AUAnimadorObjetoEscena();
	_layerMain		= new(this) AUEscenaContenedor();
	_layerDrag		= new(this) AUEscenaContenedor();
	_layerGUI		= new(this) AUEscenaContenedor();
	//
	_textEditor		= new(this) AUEscenaTextoEditableOffscreen(NBGestorTexturas::fuenteTextura("Helvetica", 12, false, false), ENTextEditorType_Literal, TRUE);
	//
	//Set drag layer
	{
		AUEscenaSprite* sprite;
		//
		sprite = new(this) AUEscenaSprite();
		sprite->redimensionar(2, 2);
		sprite->establecerMultiplicadorAlpha8(0);
		_layerDrag->agregarObjetoEscena(sprite);
		//
		sprite = new(this) AUEscenaSprite();
		sprite->redimensionar(2, 2);
		sprite->establecerMultiplicadorAlpha8(0);
		_layerDrag->agregarObjetoEscena(sprite);
		//
		_layerDrag->establecerEscuchadorTouches(this, this);
		_layerMain->agregarObjetoEscena(_layerDrag);
	}
	//Spite layer
	{
		_atlasSpriteLayer = new(this) AUEscenaContenedor();
		_layerMain->agregarObjetoEscena(_atlasSpriteLayer);
	}
	//Mesh external rules
	{
		SI32 i;
		for(i = 0; i < ENEditorAtlasRule_Count; i++){
			_atlasRules[i] = new(this) AUArregloMutable();
		}
		_atlasRulesLayer	= new(this) AUEscenaContenedor();
		_layerMain->agregarObjetoEscena(_atlasRulesLayer);
	}
	//Mesh internal grid
	{
		SI32 i;
		for(i = 0; i < ENEditorAtlasGrid_Count; i++){
			_atlasGrid[i] = new(this) AUArregloMutable();
		}
		_atlasGridLayer	= new(this) AUEscenaContenedor();
		_layerMain->agregarObjetoEscena(_atlasGridLayer);
	}
	//Atlas areas
	{
		_atlasAreas = new(this) AUArregloNativoMutableP<STEditorAtlasArea>();
		_atlasAreasSubLayer = new(this) AUEscenaContenedor();
		_atlasAreasLayer	= new(this) AUEscenaContenedor();
		_layerMain->agregarObjetoEscena(_atlasAreasSubLayer);
		_layerMain->agregarObjetoEscena(_atlasAreasLayer);
	}
	//Image list
	{
		_guiImagesList = new(this) AUIListImages();
		_guiImagesList->setListener(this);
		_layerGUI->agregarObjetoEscena(_guiImagesList);
	}
	//Buttons
	{
		_guiMenu	= new(this) AUIMenu();
		_guiMenu->setListener(this);
		_layerGUI->agregarObjetoEscena(_guiMenu);
		//
		SI32 i;
		for(i = 0; i  < ENEditorAtlasButton_Count; i++){
			switch (i) {
				
				case ENEditorAtlasButton_Save:					_guiMenu->optionAdd("SAVE"); break;
				case ENEditorAtlasButton_ExportAll:				_guiMenu->optionAdd("EXPORT ALL"); break;
				case ENEditorAtlasButton_Refresh:				_guiMenu->optionAdd("REFRESH"); break;
				case ENEditorAtlasButton_EditMesh:				_guiMenu->optionAdd("EDIT MESH"); break;
				case ENEditorAtlasButton_PerfectPixelPrefered:	_guiMenu->optionAdd("PIXEL PRECISION OFF"); break;
				case ENEditorAtlasButton_MaskPrefered:			_guiMenu->optionAdd("MASK MODE OFF"); break;
				case ENEditorAtlasButton_TextureMode:			_guiMenu->optionAdd("TEXTURE COLOR"); break;
				//case ENEditorAtlasButton_InvertColor:			_guiMenu->optionAdd("INVERT COLOR"); break;
				//case ENEditorAtlasButton_FixMasksBorders:		_guiMenu->optionAdd("FIX MASKS BORDERS"); break;
				default: _guiMenu->optionAdd("[???]"); NBASSERT(false) break;
			}
		}
	}
	//Pixel color
	{
		_colorPxPos.x = _colorPxPos.y = 0;
		_colorValue.r = _colorValue.g = _colorValue.b = _colorValue.a = 0;
		{
			_colorTxt = new(this) AUEscenaTexto(NBGestorTexturas::fuenteTextura("Helvetica", 12, true, false));
			_colorTxt->establecerAlineaciones(ENNBTextLineAlignH_Left, ENNBTextAlignV_FromBottom);
			_colorTxt->establecerMultiplicadorColor8(255, 255, 255, 255);
			_colorTxt->establecerVisible(false);
			_layerGUI->agregarObjetoEscena(_colorTxt);
		}
		{
			const float w = NBGestorEscena::anchoPulgadasAEscena(_iEscena, 0.10f);
			const float h = NBGestorEscena::altoPulgadasAEscena(_iEscena, 0.10f);
			_colorSample = new(this) AUEscenaFigura(ENEscenaFiguraTipo_PoligonoCerrado);
			_colorSample->establecerVisible(false);
			_colorSample->establecerModo(ENEscenaFiguraModo_Relleno);
			_colorSample->agregarCoordenadaLocal(0, 0);
			_colorSample->agregarCoordenadaLocal(w, 0);
			_colorSample->agregarCoordenadaLocal(w, h);
			_colorSample->agregarCoordenadaLocal(0, h);
			_layerGUI->agregarObjetoEscena(_colorSample);
			//
			_colorBorder = new(this) AUEscenaFigura(ENEscenaFiguraTipo_PoligonoCerrado);
			_colorBorder->establecerVisible(false);
			_colorBorder->establecerModo(ENEscenaFiguraModo_Borde);
			_colorBorder->agregarCoordenadaLocal(0, 0);
			_colorBorder->agregarCoordenadaLocal(w, 0);
			_colorBorder->agregarCoordenadaLocal(w, h);
			_colorBorder->agregarCoordenadaLocal(0, h);
			_layerGUI->agregarObjetoEscena(_colorBorder);
		}
	}
	//
	NBPUNTO_ESTABLECER(_workspaceCenter, 0.0f, 0.0f);
	//
	//NBGestorEscena::establecerColorFondo(_iEscena, 0.1f, 0.1f, 0.1f, 1.0f);
	NBGestorEscena::normalizaCajaProyeccionEscena(_iEscena);
	NBGestorEscena::normalizaMatricesCapasEscena(_iEscena);
	_enPrimerPlano	= false;
	this->escenaColocarEnPrimerPlano();
	//
	this->privOrganizarContenido();
	NBGestorAnimadores::agregarAnimador(this, this);
}

AUEditorAtlas::~AUEditorAtlas(){
	NBGestorAnimadores::quitarAnimador(this);
	//
	this->escenaQuitarDePrimerPlano();
	if(_animObjetos!=NULL) _animObjetos->liberar(NB_RETENEDOR_THIS); _animObjetos = NULL;
	if(_layerDrag != NULL) _layerDrag->liberar(NB_RETENEDOR_THIS); _layerDrag = NULL;
	if(_layerMain != NULL) _layerMain->liberar(NB_RETENEDOR_THIS); _layerMain = NULL;
	if(_layerGUI != NULL) _layerGUI->liberar(NB_RETENEDOR_THIS); _layerGUI = NULL;
	if(_textEditor != NULL) _textEditor->liberar(NB_RETENEDOR_THIS); _textEditor = NULL;
	//
	this->privAtlasClose();
	if(_atlasSpriteLayer != NULL) _atlasSpriteLayer->liberar(); _atlasSpriteLayer = NULL;
	if(_guiImagesList != NULL) _guiImagesList->liberar(); _guiImagesList = NULL;
	if(_guiMenu != NULL) _guiMenu->liberar(); _guiMenu = NULL;
	//Color
	{
		if(_colorTxt != NULL) _colorTxt->liberar(); _colorTxt = NULL;
		if(_colorBorder != NULL) _colorBorder->liberar(); _colorBorder = NULL;
		if(_colorSample != NULL) _colorSample->liberar(); _colorSample = NULL;
	}
	//Mesh external rules (asuming 'privMeshClose' relased all objects)
	{
		SI32 i;
		for(i = 0; i < ENEditorAtlasRule_Count; i++){
			NBASSERT(_atlasRules[i]->conteo == 0)
			_atlasRules[i]->liberar();
			_atlasRules[i] = NULL;
		}
		if(_atlasRulesLayer != NULL) _atlasRulesLayer->liberar(); _atlasRulesLayer = NULL;
	}
	//Mesh internal grid (asuming 'privMeshClose' relased all objects)
	{
		SI32 i;
		for(i = 0; i < ENEditorAtlasGrid_Count; i++){
			NBASSERT(_atlasGrid[i]->conteo == 0)
			_atlasGrid[i]->liberar();
			_atlasGrid[i] = NULL;
		}
		if(_atlasGridLayer != NULL) _atlasGridLayer->liberar(); _atlasGridLayer = NULL;
	}
	//Atlas areas (asuming 'privMeshClose' relased all objects)
	{
		NBASSERT(_atlasAreas->conteo == 0)
		_atlasAreas->liberar();
		_atlasAreas = NULL;
		if(_atlasAreasLayer != NULL) _atlasAreasLayer->liberar(); _atlasAreasLayer = NULL;
		if(_atlasAreasSubLayer != NULL) _atlasAreasSubLayer->liberar(); _atlasAreasSubLayer = NULL;
	}
}

//

bool AUEditorAtlas::escenaEnPrimerPlano(){
	return _enPrimerPlano;
}

void AUEditorAtlas::escenaColocarEnPrimerPlano(){
	if(!_enPrimerPlano){
		NBColor8 coloBlanco; NBCOLOR_ESTABLECER(coloBlanco, 255, 255, 255, 255);
		NBGestorEscena::agregarObjetoCapa(_iEscena, ENGestorEscenaGrupo_Escena, _layerMain, coloBlanco);
		NBGestorEscena::agregarObjetoCapa(_iEscena, ENGestorEscenaGrupo_GUI, _layerGUI, coloBlanco);
		//NBGestorEscena::establecerColorFondo(_iEscena, 0.1f, 0.1f, 0.1f, 1.0f);
		NBGestorEscena::establecerColorFondo(_iEscena, 255.0f / 255.0f, 191.0f / 255.0f, 37.0f / 255.0f, 1.0f);
		NBGestorEscena::agregarEscuchadorCambioPuertoVision(_iEscena, this);
		NBGestorAnimadores::agregarAnimador(this, this);
		//
		NBGestorTeclas::escuchadorEstablecer(this, this);
		this->privOrganizarContenido();
		//
		if(_animObjetos!=NULL) _animObjetos->reanudarAnimacion();
		_enPrimerPlano = true;
	}
}

void AUEditorAtlas::escenaQuitarDePrimerPlano(){
	if(_enPrimerPlano){
		NBGestorAnimadores::quitarAnimador(this);
		NBGestorEscena::quitarObjetoCapa(_iEscena, _layerMain);
		NBGestorEscena::quitarObjetoCapa(_iEscena, _layerGUI);
		NBGestorEscena::quitarEscuchadorCambioPuertoVision(_iEscena, this);
		//
		NBGestorTeclas::escuchadorRenunciar(this);
		//
		if(_animObjetos!=NULL) _animObjetos->detenerAnimacion();
		_enPrimerPlano = false;
	}
}

bool AUEditorAtlas::escenaPermitidoGirarPantalla(){
	return true;
}

bool AUEditorAtlas::escenaEstaAnimandoSalida(){
	if(_animObjetos!=NULL) return (_animObjetos->conteoAnimacionesEjecutando()!=0);
	return false;
}

void AUEditorAtlas::escenaAnimarEntrada(){
	//
}

void AUEditorAtlas::escenaAnimarSalida(){
	//
}

void AUEditorAtlas::escenaGetOrientations(UI32* dstMask, ENAppOrientationBit* dstPrefered, ENAppOrientationBit* dstToApplyOnce, BOOL* dstAllowAutoRotate){
	//
}

//

void AUEditorAtlas::appFocoExclusivoObtenido(){
	//
}

void AUEditorAtlas::appFocoExclusivoPerdido(){
	//
}

void AUEditorAtlas::appProcesarNotificacionLocal(const char* grp, const SI32 localId, const char* objTip, const SI32 objId){
	//
}

//

bool AUEditorAtlas::openFiles(AUArreglo* paths){
	bool r = false;
	const SI32 pathsCount = paths->conteo;
	if(_atlasMap != NULL && _guiEditorMesh == NULL){
		//------------------------------------
		// Add one or more files to current Atlas
		//------------------------------------
		const SI32 countAreasBefore		= _atlasMap->areasOcupadas()->conteo;
		AUArregloMutable* arrBitmaps	= new(ENMemoriaTipo_Temporal) AUArregloMutable();
		AUArregloMutable* arrExtraChuncks = new(ENMemoriaTipo_Temporal) AUArregloMutable();
		AUArregloNativoMutableP<SI32>* arrOrderedIndexes = new(ENMemoriaTipo_Temporal) AUArregloNativoMutableP<SI32>();
		AUArregloNativoMutableP<NBRectanguloUI16>* arrOrderedAreas = new(ENMemoriaTipo_Temporal) AUArregloNativoMutableP<NBRectanguloUI16>();
		//Load bitmaps
		{
			SI32 i;
			for(i = 0; i < pathsCount; i++){
				AUMapaBitsMutable* mapaBits = new(this) AUMapaBitsMutable();
				AUPngChuncksMutable* extraChuncks = new(this) AUPngChuncksMutable();
				AUCadena8* strPath = (AUCadena8*)paths->elem(i);
				if(!mapaBits->cargarDesdeArchivoPNG(strPath->str(), true, NULL, extraChuncks)){
					PRINTF_ERROR("Could not load as png file '%s'.\n", strPath->str());
					break;
				} else {
					arrBitmaps->agregarElemento(mapaBits);
					arrExtraChuncks->agregarElemento(extraChuncks);
				}
				extraChuncks->liberar();
				mapaBits->liberar();
			}
		}
		//Define bitmaps order (bigger first) and ask for space on atlas
		if(arrBitmaps->conteo == pathsCount){
			SI32 i; const SI32 count = arrBitmaps->conteo;
			while(arrOrderedIndexes->conteo < count){
				SI32 iBiggest = -1; SI32 biggestArea = 0;
				for(i = 0; i < count; i++){
					if(arrOrderedIndexes->indiceDe(i) == -1){
						AUMapaBitsMutable* mapaBits = (AUMapaBitsMutable*)arrBitmaps->elem(i);
						const NBTamanoI tamBitMap = mapaBits->tamano();
						const SI32 tamArea = tamBitMap.ancho * tamBitMap.alto;
						if(biggestArea <= tamArea){
							iBiggest = i;
							biggestArea = tamArea;
						}
					}
				}
				NBASSERT(iBiggest != -1)
				if(iBiggest == -1){
					break; //error
				} else {
					//Ask fo space on atlas
					AUCadena8* strPath = (AUCadena8*)paths->elem(iBiggest);
					AUMapaBitsMutable* mapaBits = (AUMapaBitsMutable*)arrBitmaps->elem(iBiggest);
					const NBTamanoI tamBitMap = mapaBits->tamano();
					//Get file-name start position
					SI32 posFileName = strPath->ultimoIndiceDe('/') + 1;
					if(posFileName <= 0) posFileName = strPath->ultimoIndiceDe('\\') + 1;
					//Add to atlas
					const NBRectanguloUI16 areaResv = _atlasMap->reservaEspacioConNombre(tamBitMap.ancho, tamBitMap.alto, 0, 0, &strPath->str()[posFileName]);
					if(areaResv.ancho < tamBitMap.ancho || areaResv.alto < tamBitMap.alto){
						PRINTF_ERROR("Png file (%d x %d) wont fit on existing atlas: '%s'.\n", tamBitMap.ancho, tamBitMap.alto, strPath->str());
						break;
					} else {
						PRINTF_INFO("Png file (%d x %d) fit on existing atlas: '%s'.\n", tamBitMap.ancho, tamBitMap.alto, strPath->str());
						arrOrderedAreas->agregarElemento(areaResv);
						arrOrderedIndexes->agregarElemento(iBiggest);
					}
				}
			}
		}
		//
		if(arrOrderedIndexes->conteo != pathsCount){
			//Remove areas added during verification
			SI32 i;
			for(i = _atlasMap->areasOcupadas()->conteo - 1; i >= countAreasBefore; i--){
				_atlasMap->removerAreaEnIndice(i);
			}
		} else {
			//Add new areas to atlas
			SI32 i; const SI32 count = arrOrderedIndexes->conteo;
			for(i = count - 1; i >= 0; i--){
				const SI32 iBitmap					= arrOrderedIndexes->elem(i);
				const NBRectanguloUI16 areaResv		= arrOrderedAreas->elem(i);
				AUCadena8* strPath					= (AUCadena8*)paths->elem(iBitmap);
				AUMapaBitsMutable* mapaBits			= (AUMapaBitsMutable*)arrBitmaps->elem(iBitmap);
				AUPngChuncksMutable* extraChuncks	= (AUPngChuncksMutable*)arrExtraChuncks->elem(iBitmap);
				AUAtlasMapaMutable* localAtlas		= NULL;
				AUMallaMapaMutable* localMesh		= NULL;
				//Load atlas from PNG-chunck
				if(localAtlas == NULL){
					const AUCadenaLarga8* chunckAT		= extraChuncks->getChunckById("nbAT"); //NicaraguaBinaryATlas
					if(chunckAT != NULL){
						AUAtlasMapaMutable* atlasMap = new(this) AUAtlasMapaMutable(0, 0, 0);
						AUArchivoEnBuffer* flujo = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer(&chunckAT->str()[4], chunckAT->tamano() - 4);
						flujo->lock();
						if(AUAtlasMapa::cargarXmlEn(atlasMap, flujo)){
							if(atlasMap->areasOcupadas()->conteo == 0){
								PRINTF_WARNING("Empty atlas loaded from png-chunck, ignoring.\n");
							} else {
								const NBTamanoI tamBitMap		= mapaBits->tamano();
								const NBTamanoP<UI16> tamAtlas	= atlasMap->tamano();
								if(tamAtlas.ancho == tamBitMap.ancho && tamAtlas.alto == tamBitMap.alto){
									localAtlas = atlasMap; atlasMap->retener();
									PRINTF_INFO("Atlas loaded from png-chunck.\n");
								} else {
									PRINTF_ERROR("Discarting atlas, size does not match with bitmap.\n");
								}
							}
						}
						flujo->unlock();
						flujo->liberar();
						atlasMap->liberar();
					}
				}
				//Load atlas from backup file (.png.atlas.bak)
				if(localAtlas == NULL){
					AUAtlasMapaMutable* atlasMap = new(this) AUAtlasMapaMutable(0, 0, 0);
					AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
					strTmp->agregar(strPath->str()); strTmp->agregar(".atlas.bak");
					if(AUAtlasMapa::cargarXmlEn(atlasMap, strTmp->str())){
						PRINTF_INFO("Atlas loaded from '.png.atlas.bak' file.\n");
						localAtlas = atlasMap; atlasMap->retener();
					}
					strTmp->liberar();
					atlasMap->liberar();
				}
				//Load mesh from PNG-chunck
				if(localMesh == NULL){
					const AUCadenaLarga8* chunckMH = extraChuncks->getChunckById("nbMH"); //NicaraguaBinaryMesH
					if(chunckMH != NULL){
						AUMallaMapaMutable* meshMap = new(this) AUMallaMapaMutable(1);
						AUArchivoEnBuffer* flujo = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer(&chunckMH->str()[4], chunckMH->tamano() - 4);
						flujo->lock();
						if(AUMallaMapa::cargarXmlEn(meshMap, flujo)){
							PRINTF_INFO("Mesh loaded from png-chunck.\n");
							localMesh = meshMap; meshMap->retener();
						}
						flujo->unlock();
						flujo->liberar();
						meshMap->liberar();
					}
				}
				//Load atlas from backup file (.png.mesh.bak)
				if(localMesh == NULL){
					AUMallaMapaMutable* meshMap = new(this) AUMallaMapaMutable(1);
					AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
					strTmp->agregar(strPath->str()); strTmp->agregar(".mesh.bak");
					if(AUMallaMapa::cargarXmlEn(meshMap, strTmp->str())){
						PRINTF_INFO("Mesh loaded from '.png.mesh.bak' file.\n");
						localMesh = meshMap; meshMap->retener();
					}
					strTmp->liberar();
					meshMap->liberar();
				}
				//Paste bitmap
				_atlasMapaBits->pegarMapaBitsRGBA(areaResv.x, areaResv.y, mapaBits);
				//Replace current atlas single-area with multiple-areas (if necesary)
				if(localAtlas != NULL){
					_atlasMap->removerAreaEnIndice(countAreasBefore + i);
					const AUArregloNativoP<STAtlasArea>* localAreas = localAtlas->areasOcupadas();
					SI32 i; const SI32 count = localAreas->conteo;
					for(i = 0 ; i < count; i++){
						const STAtlasArea* localArea = localAreas->elemPtr(i);
						if(localArea->nombre == NULL){
							_atlasMap->agregarAreaOcupada(areaResv.x + localArea->area.x, areaResv.y + localArea->area.y, localArea->area.ancho, localArea->area.alto, 0, NULL, localMesh);
						} else {
							_atlasMap->agregarAreaOcupada(areaResv.x + localArea->area.x, areaResv.y + localArea->area.y, localArea->area.ancho, localArea->area.alto, 0, localArea->nombre->str(), localMesh);
						}
					}
				}
				if(localAtlas != NULL) localAtlas->liberar();
				if(localMesh != NULL) localMesh->liberar();
			}
			//
			this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_SHAPES | EDITOR_ATLAS_UPDATE_MASK_BITMAPS_IF_NECESARY);
			this->privAtlasRefreshTexture();
			r = true;
		}
		//
		arrOrderedAreas->liberar();
		arrOrderedIndexes->liberar();
		arrExtraChuncks->liberar();
		arrBitmaps->liberar();
	} else {
		if(pathsCount == 1){
			//--------------------------
			//-- New atlas from one file
			//--------------------------
			AUCadena8* strPath = (AUCadena8*)paths->elem(0);
			AUMapaBitsMutable* mapaBits = new(this) AUMapaBitsMutable();
			AUPngChuncksMutable* extraChuncks = new(this) AUPngChuncksMutable();
			if(!mapaBits->cargarDesdeArchivoPNG(strPath->str(), true, NULL, extraChuncks)){
				PRINTF_ERROR("Couldnt load image file: '%s'.\n", strPath->str());
			} else {
				const NBTamanoI tamano = mapaBits->tamano();
				AUMapaBitsLienzo* lienzo = new(this) AUMapaBitsLienzo(tamano.ancho, tamano.alto);
				lienzo->pegarMapaBitsRGBA(0, 0, mapaBits);
				_atlasTexture = NBGestorTexturas::texturaDesdeMapaBits(lienzo, "", ENTexturaModoPintado_Imagen_Precisa, ENTexturaMipMap_Inhabilitado, ENTexturaOrdenV_HaciaArriba, _atlasCoordMutliplier, 1.0f);
				if(_atlasTexture == NULL){
					PRINTF_ERROR("File loaded but could not load as texture: '%s'.\n", strPath->str());
				} else {
					NBASSERT(_atlasMap == NULL)
					NBASSERT(_atlasMapaBits == NULL)
					NBASSERT(_atlasMapaBitsCustomView == NULL)
					_atlasMapaBits = lienzo; _atlasMapaBits->retener();
					PRINTF_INFO("File loaded with %d extra chuncks.\n", extraChuncks->countOfChuncks());
					_atlasMapaBitsExtraChuncks = extraChuncks; _atlasMapaBitsExtraChuncks->retener();
					_atlasTexture->retener();
					_atlasSprite = new(this) AUEscenaSprite(_atlasTexture, ENSpriteAlineaH_Der, ENSpriteAlineaV_Arr);
					_atlasFilePath = new(this) AUCadena8(strPath->str());
					_atlasSpriteLayer->agregarObjetoEscena(_atlasSprite);
					//Load bitmap properties
					{
						_atlasProps = NULL;
						//Load props from PNG-chunck
						if(_atlasProps == NULL){
							const AUCadenaLarga8* chunckBP = extraChuncks->getChunckById("nbBP"); //NicaraguaBinaryBitmapProps
							if(chunckBP != NULL){
								AUMapaBitsPropsMutable* bitmapProps = new(this) AUMapaBitsPropsMutable();
								AUArchivoEnBuffer* flujo = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer(&chunckBP->str()[4], chunckBP->tamano() - 4);
								flujo->lock();
								if(!AUMapaBitsProps::cargarXmlEn(bitmapProps, flujo)){
									PRINTF_ERROR("Could not load bitmapProps from png data chunck: (%d bytes).\n", chunckBP->tamano());
								} else {
									_atlasProps = bitmapProps; bitmapProps->retener();
									PRINTF_INFO("BitmapProps loaded from png data chunck: (%d bytes).\n", chunckBP->tamano());
								}
								flujo->unlock();
								flujo->liberar();
								bitmapProps->liberar();
							}
						}
						//Load props from backup file (.png.props.bak)
						if(_atlasProps == NULL){
							AUMapaBitsPropsMutable* bitmapProps = new(this) AUMapaBitsPropsMutable();
							AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
							strTmp->agregar(strPath->str()); strTmp->agregar(".props.bak");
							if(AUMapaBitsProps::cargarXmlEn(bitmapProps, strTmp->str())){
								PRINTF_INFO("BitmapProps loaded from '.png.props.bak' file.\n");
								_atlasProps = bitmapProps; bitmapProps->retener();
							}
							strTmp->liberar();
							bitmapProps->liberar();
						}
						//New props
						if(_atlasProps == NULL){
							_atlasProps = new(this) AUMapaBitsPropsMutable();
						}
						if(_atlasProps->mascaraAlphaPreferido()){
							//Verify if it is mask compatible
							const STBitmapPixsDesc pxDesc = _atlasMapaBits->pixelsDesc();
							if((pxDesc.mask & NB_BITMAP_PIXS_DESC_BIT_RGB) != 0 || (pxDesc.mask & NB_BITMAP_PIXS_DESC_BIT_TRANSP) != 0){
								_atlasProps->establecerMascaraAlphaPreferido(false);
							}
						}
						_guiMenu->optionUpdate(ENEditorAtlasButton_PerfectPixelPrefered, _atlasProps->precisionPixelPrefererida() ? "PIXEL PRECISON ON" : "PIXEL PRECISON OFF");
						_guiMenu->optionUpdate(ENEditorAtlasButton_MaskPrefered, _atlasProps->mascaraAlphaPreferido() ? "MASK MODE ON" : "MASK MODE OFF");
					}
					//
					const NBTamano sizeHD = _atlasTexture->tamanoHD();
					NBTamanoI sizePix; NBTAMANO_ESTABLECER(sizePix, sizeHD.ancho, sizeHD.alto)
					//Create new atlas and load from chuncks-data (if available)
					{
						_atlasMap = NULL;
						//Load atlas from PNG-chunck
						if(_atlasMap == NULL){
							const AUCadenaLarga8* chunckAT = extraChuncks->getChunckById("nbAT"); //NicaraguaBinaryATlas
							if(chunckAT != NULL){
								AUAtlasMapaMutable* atlasMap = new(this) AUAtlasMapaMutable(0, 0, 0);
								AUArchivoEnBuffer* flujo = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer(&chunckAT->str()[4], chunckAT->tamano() - 4);
								flujo->lock();
								if(!AUAtlasMapa::cargarXmlEn(atlasMap, flujo)){
									PRINTF_ERROR("Could not load atlas from png data chunck: (%d bytes).\n", chunckAT->tamano());
								} else if(atlasMap->areasOcupadas()->conteo == 0){
									PRINTF_WARNING("Empty atlas loaded from png-chunck, ignoring.\n");
								} else {
									const NBTamanoI tamBitMap		= mapaBits->tamano();
									const NBTamanoP<UI16> tamAtlas	= atlasMap->tamano();
									if(tamAtlas.ancho == tamBitMap.ancho && tamAtlas.alto == tamBitMap.alto){
										_atlasMap = atlasMap; atlasMap->retener();
										PRINTF_INFO("Atlas loaded from png data chunck: (%d bytes).\n", chunckAT->tamano());
									} else {
										PRINTF_ERROR("Discarting atlas, size does not match with bitmap.\n");
									}
								}
								flujo->unlock();
								flujo->liberar();
								atlasMap->liberar();
							}
						}
						//Load atlas from backup file (.png.atlas.bak)
						if(_atlasMap == NULL){
							AUAtlasMapaMutable* atlasMap = new(this) AUAtlasMapaMutable(0, 0, 0);
							AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
							strTmp->agregar(strPath->str()); strTmp->agregar(".atlas.bak");
							if(AUAtlasMapa::cargarXmlEn(atlasMap, strTmp->str())){
								if(atlasMap->areasOcupadas()->conteo == 0){
									PRINTF_WARNING("Empty atlas loaded from '.png.atlas.bak' file, ignoring.\n");
								} else {
									PRINTF_INFO("Atlas loaded from '.png.atlas.bak' file.\n");
									_atlasMap = atlasMap; atlasMap->retener();
								}
							}
							strTmp->liberar();
							atlasMap->liberar();
						}
						//New atlas
						if(_atlasMap == NULL){
							_atlasMap = new(this) AUAtlasMapaMutable(this->_atlasCoordMutliplier, sizePix.ancho, sizePix.alto);
						}
					}
					this->privInitRulesAndGrid(sizePix.ancho, sizePix.alto, 0.0f);
					this->privWorkspaceSetScale(_workspaceScale);
					this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_SHAPES | EDITOR_ATLAS_UPDATE_MASK_BITMAPS_IF_NECESARY);
					//_atlasMapaBits->guardarComoMapaBits("./curBitmap", false);
					//
					r = true;
				}
				lienzo->liberar();
			}
			extraChuncks->liberar();
			mapaBits->liberar();
		} else {
			//--------------------------
			//-- New atlas from multiple files
			//--------------------------
			AUArregloMutable* arrBitmaps		= new(ENMemoriaTipo_Temporal) AUArregloMutable();
			AUArregloMutable* arrExtraChuncks	= new(ENMemoriaTipo_Temporal) AUArregloMutable();
			AUArregloMutable* arrPaths			= new(ENMemoriaTipo_Temporal) AUArregloMutable();
			AUArregloNativoMutableP<SI32>* arrOrderedIndexes = new(ENMemoriaTipo_Temporal) AUArregloNativoMutableP<SI32>();
			//Load bitmaps
			{
				SI32 i;
				for(i = 0; i < pathsCount; i++){
					AUMapaBitsMutable* mapaBits = new(this) AUMapaBitsMutable();
					AUPngChuncksMutable* extraChuncks = new(this) AUPngChuncksMutable();
					AUCadena8* strPath = (AUCadena8*)paths->elem(i);
					if(!mapaBits->cargarDesdeArchivoPNG(strPath->str(), true, NULL, extraChuncks)){
						PRINTF_ERROR("Could not load as png file '%s'.\n", strPath->str());
						break;
					} else {
						const NBTamanoI tamBitMap = mapaBits->tamano();
						PRINTF_INFO("Png file (%d x %d) loaded '%s'.\n", tamBitMap.ancho, tamBitMap.alto, strPath->str());
						arrBitmaps->agregarElemento(mapaBits);
						arrExtraChuncks->agregarElemento(extraChuncks);
						arrPaths->agregarElemento(strPath);
					}
					extraChuncks->liberar();
					mapaBits->liberar();
				}
			}
			//
			if(arrBitmaps->conteo == pathsCount){
				AUAtlasMapaMutable* mapaAtlas = NULL;
				//Manually build mesh-3-horizontal
				if(mapaAtlas == NULL && pathsCount == 3){
					NBTamanoI sizeBitmaps[3];
					SI32 i;
					for(i = 0; i < 3; i++){
						sizeBitmaps[i] = ((AUMapaBitsMutable*)arrBitmaps->elem(i))->tamano();
					}
					if(sizeBitmaps[0].alto == sizeBitmaps[1].alto && sizeBitmaps[0].alto == sizeBitmaps[2].alto){ //row same height
						NBTamanoI sizeCorrected[3];
						for(i = 0; i < 3; i++){
							sizeCorrected[i] = sizeBitmaps[i];
							if((sizeCorrected[i].ancho % _atlasCoordMutliplier) != 0) sizeCorrected[i].ancho = (sizeCorrected[i].ancho / _atlasCoordMutliplier) + _atlasCoordMutliplier;
							if((sizeCorrected[i].alto % _atlasCoordMutliplier) != 0) sizeCorrected[i].alto = (sizeCorrected[i].alto / _atlasCoordMutliplier) + _atlasCoordMutliplier;
							arrOrderedIndexes->agregarElemento(i);
						}
						//
						mapaAtlas	= new(ENMemoriaTipo_Temporal) AUAtlasMapaMutable(this->_atlasCoordMutliplier, sizeCorrected[0].ancho + sizeCorrected[1].ancho + sizeCorrected[2].ancho, sizeCorrected[0].alto);
						//Row
						mapaAtlas->agregarAreaOcupada(0, 0, sizeBitmaps[0].ancho, sizeBitmaps[0].alto, 0, NULL, NULL);
						mapaAtlas->agregarAreaOcupada(sizeCorrected[0].ancho, 0, sizeBitmaps[1].ancho, sizeBitmaps[1].alto, 0, NULL, NULL);
						mapaAtlas->agregarAreaOcupada(sizeCorrected[0].ancho + sizeCorrected[1].ancho, 0, sizeBitmaps[2].ancho, sizeBitmaps[2].alto, 0, NULL, NULL);
					}
				}
				//Manually build mesh-3-vertical
				if(mapaAtlas == NULL && pathsCount == 3){
					NBTamanoI sizeBitmaps[3];
					SI32 i;
					for(i = 0; i < 3; i++){
						sizeBitmaps[i] = ((AUMapaBitsMutable*)arrBitmaps->elem(i))->tamano();
					}
					if(sizeBitmaps[0].ancho == sizeBitmaps[1].ancho && sizeBitmaps[0].ancho == sizeBitmaps[2].ancho){ //column same height
						NBTamanoI sizeCorrected[3];
						for(i = 0; i < 3; i++){
							sizeCorrected[i] = sizeBitmaps[i];
							if((sizeCorrected[i].ancho % _atlasCoordMutliplier) != 0) sizeCorrected[i].ancho = (sizeCorrected[i].ancho / _atlasCoordMutliplier) + _atlasCoordMutliplier;
							if((sizeCorrected[i].alto % _atlasCoordMutliplier) != 0) sizeCorrected[i].alto = (sizeCorrected[i].alto / _atlasCoordMutliplier) + _atlasCoordMutliplier;
							arrOrderedIndexes->agregarElemento(i);
						}
						//
						mapaAtlas	= new(ENMemoriaTipo_Temporal) AUAtlasMapaMutable(this->_atlasCoordMutliplier, sizeCorrected[0].ancho, sizeCorrected[0].alto + sizeCorrected[1].alto + sizeCorrected[2].alto);
						//Column
						mapaAtlas->agregarAreaOcupada(0, 0, sizeBitmaps[0].ancho, sizeBitmaps[0].alto, 0, NULL, NULL);
						mapaAtlas->agregarAreaOcupada(0, sizeCorrected[0].alto, sizeBitmaps[1].ancho, sizeBitmaps[1].alto, 0, NULL, NULL);
						mapaAtlas->agregarAreaOcupada(0, sizeCorrected[0].alto + sizeCorrected[1].alto, sizeBitmaps[2].ancho, sizeBitmaps[2].alto, 0, NULL, NULL);
					}
				}
				//Manually build mesh-9
				if(mapaAtlas == NULL && pathsCount == 9){
					NBTamanoI sizeBitmaps[9];
					SI32 i;
					for(i = 0; i < 9; i++){
						sizeBitmaps[i] = ((AUMapaBitsMutable*)arrBitmaps->elem(i))->tamano();
					}
					if(sizeBitmaps[0].ancho == sizeBitmaps[3].ancho && sizeBitmaps[0].ancho == sizeBitmaps[6].ancho		//left-column same witdh
					   && sizeBitmaps[1].ancho == sizeBitmaps[7].ancho													//central-column same witdh
					   && sizeBitmaps[2].ancho == sizeBitmaps[5].ancho && sizeBitmaps[2].ancho == sizeBitmaps[8].ancho	//right-column same witdh
					   //
					   && sizeBitmaps[4].ancho <= sizeBitmaps[1].ancho && sizeBitmaps[4].alto <= sizeBitmaps[3].alto	//Center fits
					   //
					   && sizeBitmaps[0].alto == sizeBitmaps[1].alto && sizeBitmaps[0].alto == sizeBitmaps[2].alto		//top-row same height
					   && sizeBitmaps[3].alto == sizeBitmaps[5].alto													//central-row same height
					   && sizeBitmaps[6].alto == sizeBitmaps[7].alto && sizeBitmaps[6].alto == sizeBitmaps[8].alto		//bottom-row same height
					   ){
						NBTamanoI sizeCorrected[9];
						for(i = 0; i < 9; i++){
							sizeCorrected[i] = sizeBitmaps[i];
							if((sizeCorrected[i].ancho % _atlasCoordMutliplier) != 0) sizeCorrected[i].ancho = (sizeCorrected[i].ancho / _atlasCoordMutliplier) + _atlasCoordMutliplier;
							if((sizeCorrected[i].alto % _atlasCoordMutliplier) != 0) sizeCorrected[i].alto = (sizeCorrected[i].alto / _atlasCoordMutliplier) + _atlasCoordMutliplier;
							arrOrderedIndexes->agregarElemento(i);
						}
						//
						mapaAtlas	= new(ENMemoriaTipo_Temporal) AUAtlasMapaMutable(this->_atlasCoordMutliplier, sizeCorrected[0].ancho + sizeCorrected[1].ancho + sizeCorrected[2].ancho, sizeCorrected[0].alto + sizeCorrected[3].alto + sizeCorrected[6].alto);
						//First-row
						mapaAtlas->agregarAreaOcupada(0, 0, sizeBitmaps[0].ancho, sizeBitmaps[0].alto, 0, NULL, NULL);
						mapaAtlas->agregarAreaOcupada(sizeCorrected[0].ancho, 0, sizeBitmaps[1].ancho, sizeBitmaps[1].alto, 0, NULL, NULL);
						mapaAtlas->agregarAreaOcupada(sizeCorrected[0].ancho + sizeCorrected[1].ancho, 0, sizeBitmaps[2].ancho, sizeBitmaps[2].alto, 0, NULL, NULL);
						//Second-row
						mapaAtlas->agregarAreaOcupada(0, sizeCorrected[0].alto, sizeBitmaps[3].ancho, sizeBitmaps[3].alto, 0, NULL, NULL);
						mapaAtlas->agregarAreaOcupada(sizeCorrected[3].ancho, sizeCorrected[0].alto, sizeBitmaps[4].ancho, sizeBitmaps[4].alto, 0, NULL, NULL);
						mapaAtlas->agregarAreaOcupada(sizeCorrected[3].ancho + sizeCorrected[1].ancho, sizeCorrected[0].alto, sizeBitmaps[5].ancho, sizeBitmaps[5].alto, 0, NULL, NULL);
						//Third-row
						mapaAtlas->agregarAreaOcupada(0, sizeCorrected[0].alto + sizeCorrected[3].alto, sizeBitmaps[6].ancho, sizeBitmaps[6].alto, 0, NULL, NULL);
						mapaAtlas->agregarAreaOcupada(sizeCorrected[6].ancho, sizeCorrected[0].alto + sizeCorrected[3].alto, sizeBitmaps[7].ancho, sizeBitmaps[7].alto, 0, NULL, NULL);
						mapaAtlas->agregarAreaOcupada(sizeCorrected[6].ancho + sizeCorrected[7].ancho, sizeCorrected[0].alto + sizeCorrected[3].alto, sizeBitmaps[8].ancho, sizeBitmaps[8].alto, 0, NULL, NULL);
					}
				}
				//Autodetect best minumal-size for atlas
				if(mapaAtlas == NULL){
					//Order bitmaps
					if(arrBitmaps->conteo == pathsCount){
						SI32 i; const SI32 count = arrBitmaps->conteo;
						while(arrOrderedIndexes->conteo < count){
							SI32 iBiggest = -1; SI32 biggestArea = 0;
							for(i = 0; i < count; i++){
								if(arrOrderedIndexes->indiceDe(i) == -1){
									AUMapaBitsMutable* mapaBits = (AUMapaBitsMutable*)arrBitmaps->elem(i);
									const NBTamanoI tamBitMap = mapaBits->tamano();
									const SI32 tamArea = tamBitMap.ancho * tamBitMap.alto;
									if(biggestArea <= tamArea){
										iBiggest = i;
										biggestArea = tamArea;
									}
								}
							}
							NBASSERT(iBiggest != -1)
							if(iBiggest == -1){
								break; //error
							} else {
								arrOrderedIndexes->agregarElemento(iBiggest);
							}
						}
					}
					//
					SI32 minFitWidth = 64, minFitHeight = 64; const SI32 maxAllowedSize = 4096; bool noFit = true; bool nextGrowthIsWidth = true;
					while(minFitWidth <= maxAllowedSize && minFitHeight <= maxAllowedSize && noFit){
						noFit		= false;
						mapaAtlas	= new(ENMemoriaTipo_Temporal) AUAtlasMapaMutable(this->_atlasCoordMutliplier, minFitWidth, minFitHeight);
						SI32 i; const SI32 count = arrOrderedIndexes->conteo;
						for(i = 0; i < count; i++){
							const SI32 iBitmap			= arrOrderedIndexes->elem(i);
							AUMapaBitsMutable* mapaBits	= (AUMapaBitsMutable*)arrBitmaps->elem(iBitmap);
							const NBTamanoI tamBitMap	= mapaBits->tamano();
							const NBRectanguloUI16 r	= mapaAtlas->reservaEspacioConNombre(tamBitMap.ancho, tamBitMap.alto, 0, 0, NULL);
							if(r.ancho < tamBitMap.ancho || r.alto < tamBitMap.alto){
								if(nextGrowthIsWidth){
									minFitWidth += _atlasCoordMutliplier;
								} else {
									minFitHeight += _atlasCoordMutliplier;
								}
								nextGrowthIsWidth = !nextGrowthIsWidth;
								noFit = true;
								break;
							}
						}
						if(noFit){
							mapaAtlas->liberar(NB_RETENEDOR_THIS);
						}
					}
					if(mapaAtlas == NULL){
						PRINTF_ERROR("Set of images wont fit on a atlas with limit %d.\n", maxAllowedSize);
					}
				}
				if(mapaAtlas == NULL){
					PRINTF_ERROR("Could not create atlas for set of images.\n");
				} else {
					//Remove extra-size
					NBCajaAABBP<UI16> cajaAreas = mapaAtlas->cajaDeAreas();
					if((cajaAreas.xMax % _atlasCoordMutliplier) != 0) { cajaAreas.xMax = (cajaAreas.xMax / _atlasCoordMutliplier) + _atlasCoordMutliplier; }
					if((cajaAreas.yMax % _atlasCoordMutliplier) != 0) { cajaAreas.yMax = (cajaAreas.yMax / _atlasCoordMutliplier) + _atlasCoordMutliplier; }
					//
					if(!this->privAtlasNew(cajaAreas.xMax, cajaAreas.yMax)){
						PRINTF_ERROR("Could not init atlas for size (%d x %d).\n", (SI32)cajaAreas.xMax, (SI32)cajaAreas.yMax);
					} else {
						mapaAtlas->establecerTamano(cajaAreas.xMax, cajaAreas.yMax);
						//Add new areas to atlas
						SI32 i; const SI32 count = arrOrderedIndexes->conteo;
						for(i = count - 1; i >= 0; i--){
							const SI32 iBitmap					= arrOrderedIndexes->elem(i);
							const NBRectanguloUI16 areaResv		= mapaAtlas->areaEnIndice(i);
							AUCadena8* strPath					= (AUCadena8*)arrPaths->elem(iBitmap);
							AUMapaBitsMutable* mapaBits			= (AUMapaBitsMutable*)arrBitmaps->elem(iBitmap);
							AUPngChuncksMutable* extraChuncks	= (AUPngChuncksMutable*)arrExtraChuncks->elem(iBitmap);
							AUAtlasMapaMutable* localAtlas		= NULL;
							AUMallaMapaMutable* localMesh		= NULL;
							//Load atlas from PNG-chunck
							if(localAtlas == NULL){
								const AUCadenaLarga8* chunckAT		= extraChuncks->getChunckById("nbAT"); //NicaraguaBinaryATlas
								if(chunckAT != NULL){
									AUAtlasMapaMutable* atlasMap	= new(this) AUAtlasMapaMutable(0, 0, 0);
									AUArchivoEnBuffer* flujo = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer(&chunckAT->str()[4], chunckAT->tamano() - 4);
									flujo->lock();
									if(AUAtlasMapa::cargarXmlEn(atlasMap, flujo)){
										if(atlasMap->areasOcupadas()->conteo == 0){
											PRINTF_WARNING("Empty atlas loaded from png-chunck, ignoring.\n");
										} else {
											const NBTamanoI tamBitMap		= mapaBits->tamano();
											const NBTamanoP<UI16> tamAtlas	= atlasMap->tamano();
											if(tamAtlas.ancho == tamBitMap.ancho && tamAtlas.alto == tamBitMap.alto){
												localAtlas = atlasMap; atlasMap->retener();
												PRINTF_INFO("Atlas loaded from png-chunck.\n");
											} else {
												PRINTF_ERROR("Discarting atlas, size does not match with bitmap.\n");
											}
										}
									}
									flujo->unlock();
									flujo->liberar();
									atlasMap->liberar();
								}
							}
							//Load atlas from backup file (.png.atlas.bak)
							if(localAtlas == NULL){
								AUAtlasMapaMutable* atlasMap = new(this) AUAtlasMapaMutable(0, 0, 0);
								AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
								strTmp->agregar(strPath->str()); strTmp->agregar(".atlas.bak");
								if(AUAtlasMapa::cargarXmlEn(atlasMap, strTmp->str())){
									if(atlasMap->areasOcupadas()->conteo == 0){
										PRINTF_WARNING("Empty atlas loaded from '.png.atlas.bak' file, ignoring.\n");
									} else {
										PRINTF_INFO("Atlas loaded from '.png.atlas.bak' file.\n");
										localAtlas = atlasMap; atlasMap->retener();
									}
								}
								strTmp->liberar();
								atlasMap->liberar();
							}
							//Load mesh from PNG-chunck
							if(localMesh == NULL){
								const AUCadenaLarga8* chunckMH = extraChuncks->getChunckById("nbMH"); //NicaraguaBinaryMesH
								if(chunckMH != NULL){
									AUMallaMapaMutable* meshMap = new(this) AUMallaMapaMutable(1);
									AUArchivoEnBuffer* flujo = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer(&chunckMH->str()[4], chunckMH->tamano() - 4);
									flujo->lock();
									if(AUMallaMapa::cargarXmlEn(meshMap, flujo)){
										PRINTF_INFO("Mesh loaded from png-chunck.\n");
										localMesh = meshMap; meshMap->retener();
									}
									flujo->unlock();
									flujo->liberar();
									meshMap->liberar();
								}
							}
							//Load atlas from backup file (.png.mesh.bak)
							if(localMesh == NULL){
								AUMallaMapaMutable* meshMap = new(this) AUMallaMapaMutable(1);
								AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
								strTmp->agregar(strPath->str()); strTmp->agregar(".mesh.bak");
								if(AUMallaMapa::cargarXmlEn(meshMap, strTmp->str())){
									PRINTF_INFO("Mesh loaded from '.png.mesh.bak' file.\n");
									localMesh = meshMap; meshMap->retener();
								}
								strTmp->liberar();
								meshMap->liberar();
							}
							//Paste bitmap
							_atlasMapaBits->pegarMapaBitsRGBA(areaResv.x, areaResv.y, mapaBits);
							//Replace current atlas single-area with multiple-areas (if necesary)
							if(localAtlas == NULL){
								//Get file-name start position
								SI32 posFileName = strPath->ultimoIndiceDe('/') + 1;
								if(posFileName <= 0) posFileName = strPath->ultimoIndiceDe('\\') + 1;
								//Update name of the area
								mapaAtlas->establecerAreaNombreEnIndice(i, &strPath->str()[posFileName]);
								mapaAtlas->establecerAreaMallaEnIndice(i, localMesh);
							} else {
								mapaAtlas->removerAreaEnIndice(i);
								const AUArregloNativoP<STAtlasArea>* localAreas = localAtlas->areasOcupadas();
								SI32 i; const SI32 count = localAreas->conteo;
								for(i = 0 ; i < count; i++){
									//Get file-name start position
									SI32 posFileName = strPath->ultimoIndiceDe('/') + 1;
									if(posFileName <= 0) posFileName = strPath->ultimoIndiceDe('\\') + 1;
									//Add area
									STAtlasArea* localArea = localAreas->elemPtr(i);
									mapaAtlas->agregarAreaOcupada(areaResv.x + localArea->area.x, areaResv.y + localArea->area.y, localArea->area.ancho, localArea->area.alto, 0, &strPath->str()[posFileName], localArea->mallaMapa);
								}
							}
							if(localMesh != NULL) localMesh->liberar();
							if(localAtlas != NULL) localAtlas->liberar();
						}
						//Set atlas
						if(_atlasMap != NULL) _atlasMap->liberar();
						_atlasMap = mapaAtlas; mapaAtlas->retener();
						//
						this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_SHAPES | EDITOR_ATLAS_UPDATE_MASK_BITMAPS_IF_NECESARY);
						this->privAtlasRefreshTexture();
						r = true;
					}
					mapaAtlas->liberar();
				}
			}
			//
			arrOrderedIndexes->liberar();
			arrPaths->liberar();
			arrExtraChuncks->liberar();
			arrBitmaps->liberar();
		}
	}
	return r;
}

bool AUEditorAtlas::privAtlasNew(const SI32 width, const SI32 height){
	bool r = false;
	if(width > 0 && height > 0){
		AUMapaBitsLienzo* lienzo = new(this) AUMapaBitsLienzo(width, height);
		lienzo->formatearLienzo(255, 255, 255, 0);
		_atlasTexture = NBGestorTexturas::texturaDesdeMapaBits(lienzo, "", ENTexturaModoPintado_Imagen_Precisa, ENTexturaMipMap_Inhabilitado, ENTexturaOrdenV_HaciaArriba, _atlasCoordMutliplier, 1.0f);
		if(_atlasTexture == NULL){
			PRINTF_ERROR("Could not create texture for size(%d x %d).\n", width, height);
		} else {
			NBASSERT(_atlasProps == NULL)
			NBASSERT(_atlasMap == NULL)
			NBASSERT(_atlasMapaBits == NULL)
			NBASSERT(_atlasMapaBitsCustomView == NULL)
			_atlasMapaBits = lienzo; _atlasMapaBits->retener();
			_atlasMapaBitsExtraChuncks = new(this) AUPngChuncksMutable();
			_atlasTexture->retener();
			_atlasSprite = new(this) AUEscenaSprite(_atlasTexture, ENSpriteAlineaH_Der, ENSpriteAlineaV_Arr);
			_atlasFilePath = new(this) AUCadena8("");
			_atlasSpriteLayer->agregarObjetoEscena(_atlasSprite);
			//
			const NBTamano sizeHD = _atlasTexture->tamanoHD();
			NBTamanoI sizePix; NBTAMANO_ESTABLECER(sizePix, sizeHD.ancho, sizeHD.alto)
			//
			_atlasProps = new(this) AUMapaBitsPropsMutable();
			_guiMenu->optionUpdate(ENEditorAtlasButton_PerfectPixelPrefered, _atlasProps->precisionPixelPrefererida() ? "PIXEL PRECISON ON" : "PIXEL PRECISON OFF");
			_guiMenu->optionUpdate(ENEditorAtlasButton_MaskPrefered, _atlasProps->mascaraAlphaPreferido() ? "MASK MODE ON" : "MASK MODE OFF");
			//Create new atlas and load from chuncks-data (if available)
			_atlasMap = new(this) AUAtlasMapaMutable(this->_atlasCoordMutliplier, sizePix.ancho, sizePix.alto);
			//
			this->privInitRulesAndGrid(sizePix.ancho, sizePix.alto, 0.0f);
			this->privWorkspaceSetScale(_workspaceScale);
			this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_SHAPES | EDITOR_ATLAS_UPDATE_MASK_BITMAPS_IF_NECESARY);
			//
			r = true;
		}
		lienzo->liberar();
	}
	return r;
}

void AUEditorAtlas::privInitRulesAndGrid(const SI32 width, const SI32 height, const SI32 marginFromMesh){
	//Add horizontal extenal rules / internal grid
	SI32 p;
	for(p = 0; p < width; p++){
		SI32 lineLen; NBColor8 ruleColor, gridColor; AUEscenaFigura* fig;
		if((p % _atlasCoordMutliplier) == 0){
			lineLen = 8;
			NBCOLOR_ESTABLECER(ruleColor, 0, 255, 0, 255)
			NBCOLOR_ESTABLECER(gridColor, 150, 150, 150, 255)
		} else if((p % (_atlasCoordMutliplier / 2)) == 0){
			lineLen = 4;
			NBCOLOR_ESTABLECER(ruleColor, 0, 155, 0, 255)
			NBCOLOR_ESTABLECER(gridColor, 100, 100, 100, 255)
		} else {
			lineLen = 2;
			NBCOLOR_ESTABLECER(ruleColor, 0, 55, 0, 255)
			NBCOLOR_ESTABLECER(gridColor, 50, 50, 50, 255)
		}
		//Up
		fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
		fig->agregarCoordenadaLocal(0.0f, 0.0f);
		fig->agregarCoordenadaLocal(0.0f, lineLen);
		fig->establecerTraslacion(p, height + marginFromMesh);
		fig->establecerMultiplicadorColor8(ruleColor);
		_atlasRules[ENEditorAtlasRule_Top]->agregarElemento(fig);
		_atlasRulesLayer->agregarObjetoEscena(fig);
		fig->liberar();
		//Down
		fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
		fig->agregarCoordenadaLocal(0.0f, 0.0f);
		fig->agregarCoordenadaLocal(0.0f, -lineLen);
		fig->establecerTraslacion(p, -marginFromMesh);
		fig->establecerMultiplicadorColor8(ruleColor);
		_atlasRules[ENEditorAtlasRule_Bottom]->agregarElemento(fig);
		_atlasRulesLayer->agregarObjetoEscena(fig);
		fig->liberar();
		//Vertical (grid)
		fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
		fig->agregarCoordenadaLocal(0.0f, 0.0f);
		fig->agregarCoordenadaLocal(0.0f, height);
		fig->establecerTraslacion(p, 0.0f);
		fig->establecerMultiplicadorColor8(gridColor);
		_atlasGrid[ENEditorAtlasGrid_CutV]->agregarElemento(fig);
		_atlasGridLayer->agregarObjetoEscena(fig);
		fig->liberar();
	}
	//Add vertical external rules / internal grid
	for(p = 0; p < height; p++){
		SI32 lineLen; NBColor8 ruleColor, gridColor; AUEscenaFigura* fig;
		if((p % _atlasCoordMutliplier) == 0){
			lineLen = 8;
			NBCOLOR_ESTABLECER(ruleColor, 0, 255, 0, 255)
			NBCOLOR_ESTABLECER(gridColor, 150, 150, 150, 255)
		} else if((p % (_atlasCoordMutliplier / 2)) == 0){
			lineLen = 4;
			NBCOLOR_ESTABLECER(ruleColor, 0, 155, 0, 255)
			NBCOLOR_ESTABLECER(gridColor, 100, 100, 100, 255)
		} else {
			lineLen = 2;
			NBCOLOR_ESTABLECER(ruleColor, 0, 55, 0, 255)
			NBCOLOR_ESTABLECER(gridColor, 50, 50, 50, 255)
		}
		//Right
		fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
		fig->agregarCoordenadaLocal(0.0f, 0.0f);
		fig->agregarCoordenadaLocal(lineLen, 0.0f);
		fig->establecerTraslacion(width + marginFromMesh, p);
		fig->establecerMultiplicadorColor8(ruleColor);
		_atlasRules[ENEditorAtlasRule_Right]->agregarElemento(fig);
		_atlasRulesLayer->agregarObjetoEscena(fig);
		fig->liberar();
		//Left
		fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
		fig->agregarCoordenadaLocal(0.0f, 0.0f);
		fig->agregarCoordenadaLocal(-lineLen, 0.0f);
		fig->establecerTraslacion(-marginFromMesh, p);
		fig->establecerMultiplicadorColor8(ruleColor);
		_atlasRules[ENEditorAtlasRule_Left]->agregarElemento(fig);
		_atlasRulesLayer->agregarObjetoEscena(fig);
		fig->liberar();
		//Horizontal (grid)
		fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
		fig->agregarCoordenadaLocal(0.0f, 0.0f);
		fig->agregarCoordenadaLocal(width, 0.0f);
		fig->establecerTraslacion(0.0f, p);
		fig->establecerMultiplicadorColor8(gridColor);
		_atlasGrid[ENEditorAtlasGrid_CutH]->agregarElemento(fig);
		_atlasGridLayer->agregarObjetoEscena(fig);
		fig->liberar();
	}
}

void AUEditorAtlas::privAtlasClose(){
	if(_atlasFilePath != NULL) _atlasFilePath->liberar(); _atlasFilePath = NULL;
	if(_atlasMapaBits != NULL) _atlasMapaBits->liberar(); _atlasMapaBits = NULL;
	if(_atlasMapaBitsCustomView != NULL) _atlasMapaBitsCustomView->liberar(); _atlasMapaBitsCustomView = NULL;
	if(_atlasMapaBitsExtraChuncks != NULL) _atlasMapaBitsExtraChuncks->liberar(); _atlasMapaBitsExtraChuncks = NULL;
	if(_atlasTexture != NULL) _atlasTexture->liberar(); _atlasTexture = NULL;
	if(_atlasProps != NULL) _atlasProps->liberar(); _atlasProps = NULL;
	if(_atlasMap != NULL) _atlasMap->liberar(); _atlasMap = NULL;
	if(_atlasSprite != NULL){
		AUEscenaContenedor* parent = (AUEscenaContenedor*)_atlasSprite->contenedor();
		if(parent != NULL){
			parent->quitarObjetoEscena(_atlasSprite);
		}
		_atlasSprite->liberar();
		_atlasSprite = NULL;
	}
	//Mesh extenral rules
	{
		SI32 i, j;
		for(i = 0; i < ENEditorAtlasRule_Count; i++){
			const SI32 linesCount = _atlasRules[i]->conteo;
			for(j = 0; j < linesCount; j++){
				AUEscenaFigura* fig = (AUEscenaFigura*)_atlasRules[i]->elem(j);
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != NULL){
					parent->quitarObjetoEscena(fig);
				}
			}
			_atlasRules[i]->vaciar();
		}
	}
	//Mesh internal grid
	{
		SI32 i, j;
		for(i = 0; i < ENEditorAtlasGrid_Count; i++){
			const SI32 linesCount = _atlasGrid[i]->conteo;
			for(j = 0; j < linesCount; j++){
				AUEscenaFigura* fig = (AUEscenaFigura*)_atlasGrid[i]->elem(j);
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != NULL){
					parent->quitarObjetoEscena(fig);
				}
			}
			_atlasGrid[i]->vaciar();
		}
	}
	//Atlas areas
	{
		SI32 i, j;
		for(i = 0; i < _atlasAreas->conteo; i++){
			STEditorAtlasArea* area = _atlasAreas->elemPtr(i);
			AUEscenaContenedor* parent = (AUEscenaContenedor*)area->visualArea->contenedor();
			if(parent != NULL) { parent->quitarObjetoEscena(area->visualArea); }
			area->visualArea->liberar();
			for(j = 0; j < ENEditorAtlasRule_Count; j++){
				AUEscenaContenedor* parent = (AUEscenaContenedor*)area->visualLimit[j]->contenedor();
				if(parent != NULL) { parent->quitarObjetoEscena(area->visualLimit[j]); }
				area->visualLimit[j]->liberar();
			}
			if(area->bitMap != NULL) area->bitMap->liberar();
		}
		_atlasAreas->vaciar();
	}
	//
	this->privMovClear(false);
	//this->privAtlasRefreshTexture(); //Only necesary if saving before finalizing this object
}

bool AUEditorAtlas::privAtlasSave(){
	bool r = false;
	AUCadena8* strPath = NULL;
	//Use load-name
	if(strPath == NULL && _atlasFilePath != NULL){
		if(_atlasFilePath->tamano() != 0){
			strPath = _atlasFilePath; _atlasFilePath->retener();
		}
	}
	//Generate a temporary-name
	if(strPath == NULL){
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal)AUCadenaMutable8();
		SI32 i = 0; const SI32 iMax = 64;
		do {
			strTmp->vaciar();
			strTmp->agregar("./_atlas_"); strTmp->agregarNumerico(i); strTmp->agregar(".png");
			FILE* arch = fopen(strTmp->str(), "rb");
			if(arch == NULL){
				if(_atlasFilePath != NULL) _atlasFilePath->liberar();
				_atlasFilePath = strTmp; strTmp->retener();
				strPath = strTmp; strTmp->retener();
			} else {
				strTmp->vaciar();
				fclose(arch);
			}
			i++;
		} while(strTmp->tamano() == 0 && i < iMax);
		strTmp->liberar();
	}
	//Save
	if(strPath != NULL && _atlasMap != NULL && _atlasMapaBits != NULL){
		if(strPath->tamano() > 0){
			bool exito = true;
			//Atlas Chunck
			if(exito){
				AUArchivoEnBuffer* flujoAtlas = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer();
				flujoAtlas->lock();
				if(!AUAtlasMapa::guardarEnXml(_atlasMap, flujoAtlas)){
					PRINTF_ERROR("Could not generate xml for atlas.\n");
					exito = false;
				} else {
					const AUCadena8* buffStream = flujoAtlas->buffer();
					_atlasMapaBitsExtraChuncks->setChunck("nbAT", (const BYTE*)buffStream->str(), buffStream->tamano()); //NicaraguaBinaryATlas
					PRINTF_INFO("Atlas saved into '%s': (%d bytes in xml).\n", strPath->str(), buffStream->tamano());
					//Save atlas backup file
					AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8(strPath->str());
					strTmp->agregar(".atlas.bak");
					AUArchivo* flujo = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, strTmp->str(), ENArchivoModo_SoloEscritura);
					if(flujo == NULL){
						PRINTF_ERROR("Could not open file for writting: '%s'.\n", strTmp->str());
					} else {
						flujo->lock();
						if(!AUAtlasMapa::guardarEnXml(_atlasMap, flujo)){
							PRINTF_ERROR("Could not generate xml of atlas.\n");
						} else {
							PRINTF_INFO("Atlas-backup saved into '%s'.\n", strTmp->str());
						}
						flujo->unlock();
						flujo->cerrar();
					}
					strTmp->liberar();
				}
				flujoAtlas->unlock();
				flujoAtlas->liberar();
			}
			//Properties Chunck
			if(exito){
				AUArchivoEnBuffer* flujoProps = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer();
				flujoProps->lock();
				if(!AUMapaBitsProps::guardarEnXml(_atlasProps, flujoProps)){
					PRINTF_ERROR("Could not generate xml for props.\n");
					exito = false;
				} else {
					const AUCadena8* buffStream = flujoProps->buffer();
					_atlasMapaBitsExtraChuncks->setChunck("nbBP", (const BYTE*)buffStream->str(), buffStream->tamano()); //NicaraguaBinaryBitmapProperties
					PRINTF_INFO("Properties saved into '%s': (%d bytes in xml).\n", strPath->str(), buffStream->tamano());
					//Save atlas backup file
					AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8(strPath->str());
					strTmp->agregar(".props.bak");
					AUArchivo* flujo = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, strTmp->str(), ENArchivoModo_SoloEscritura);
					if(flujo == NULL){
						PRINTF_ERROR("Could not open file for writting: '%s'.\n", strTmp->str());
					} else {
						flujo->lock();
						if(!AUMapaBitsProps::guardarEnXml(_atlasProps, flujo)){
							PRINTF_ERROR("Could not generate xml of props.\n");
						} else {
							PRINTF_INFO("Props-backup saved into '%s'.\n", strTmp->str());
						}
						flujo->unlock();
						flujo->cerrar();
					}
					strTmp->liberar();
				}
				flujoProps->unlock();
				flujoProps->liberar();
			}
			//
			if(exito){
				if(!_atlasMapaBits->guardarComoPNG(strPath->str(), false, NULL, NULL, NULL, _atlasMapaBitsExtraChuncks)){
					PRINTF_ERROR("Could not save to '%s'.\n", strPath->str());
				} else {
					r = true;
				}
			}
		}
	}
	if(strPath != NULL){
		strPath->liberar();
	}
	return r;
}

bool AUEditorAtlas::privAtlasExportAll(){
	bool r = false;
	AUCadena8* strPath = NULL;
	//Use load-name
	if(strPath == NULL && _atlasFilePath != NULL){
		if(_atlasFilePath->tamano() != 0){
			strPath = _atlasFilePath; _atlasFilePath->retener();
		}
	}
	//Generate a temporary-name
	if(strPath == NULL){
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal)AUCadenaMutable8();
		SI32 i = 0; const SI32 iMax = 64;
		do {
			strTmp->vaciar();
			strTmp->agregar("./_atlas_"); strTmp->agregarNumerico(i); strTmp->agregar(".png");
			FILE* arch = fopen(strTmp->str(), "rb");
			if(arch == NULL){
				if(_atlasFilePath != NULL) _atlasFilePath->liberar();
				_atlasFilePath = strTmp; strTmp->retener();
				strPath = strTmp; strTmp->retener();
			} else {
				strTmp->vaciar();
				fclose(arch);
			}
			i++;
		} while(strTmp->tamano() == 0 && i < iMax);
		strTmp->liberar();
	}
	//Save
	if(strPath != NULL && _atlasMap != NULL && _atlasMapaBits != NULL){
		if(strPath->tamano() > 4){
			const AUArregloNativoP<STAtlasArea>* aa = _atlasMap->areasOcupadas();
			SI32 i; for(i = 0; i < aa->conteo; i++){
				const STAtlasArea* a = aa->elemPtr(i);
				const char* name = a->nombre->str();
				if(!NBString_strIsEmpty(name) && a->area.ancho > 0 && a->area.alto > 0){
					STNBString fpath;
					NBString_init(&fpath);
					NBString_concatBytes(&fpath, strPath->str(), strPath->tamano() - 4);
					NBString_concat(&fpath, "_");
					NBString_concat(&fpath, name);
					NBString_concat(&fpath, &strPath->str()[strPath->tamano() - 4]);
					{
						AUMapaBits* bmp = new(this) AUMapaBits(_atlasMapaBits, a->area.x, a->area.y, a->area.ancho, a->area.alto);
						//ToDo: include meshMap
						if(!bmp->guardarComoPNG(fpath.str, false, NULL, NULL, NULL, NULL)){
							PRINTF_ERROR("Could not save to '%s'.\n", fpath.str);
						} else {
							r = true;
						}
						bmp->liberar();
						bmp = NULL;
					}
					NBString_release(&fpath);
				}
			}
		}
	}
	if(strPath != NULL){
		strPath->liberar();
	}
	return r;
}

void AUEditorAtlas::privAtlasRefreshTexture(){
	if(_atlasTexture != NULL){
		_atlasTexture->liberar();
		_atlasTexture = NULL;
	}
	if(_atlasMapaBitsCustomView != NULL){
		_atlasMapaBitsCustomView->liberar();
		_atlasMapaBitsCustomView = NULL;
	}
	/*ENEditorAtlasTextueMode_ColorOnly
	, ENEditorAtlasTextueMode_AlphaOnly
	, ENEditorAtlasTextueMode_RedOnly
	, ENEditorAtlasTextueMode_GreenOnly
	, ENEditorAtlasTextueMode_BlueOnly
	, ENEditorAtlasTextueMode_Gray*/
	switch (_atlasTextureMode) {
		case ENEditorAtlasTextueMode_ColorOnly:
			{
				PRINTF_INFO("Texture mode: COLOR-ONLY.\n");
				_atlasMapaBitsCustomView = new(this) AUMapaBits(_atlasMapaBits);
				BYTE* data = _atlasMapaBitsCustomView->datos();
				const MapaDeBitsDesciptor props = _atlasMapaBitsCustomView->propiedades(); NBASSERT(props.color == COLOR_RGBA8)
				SI32 y; const SI32 lines = props.alto; const SI32 cols = props.ancho;
				for(y = 0; y < lines; y++){
					BYTE* posAtLine = &data[(SI32)props.bytesPorLinea * y];
					const BYTE* posAtLineNext = posAtLine + ((props.bitsPorPixel / 8) * cols);
					while(posAtLine < posAtLineNext){
						posAtLine[3] = 255;
						posAtLine += 4;
					}
				}
			}
			break;
		case ENEditorAtlasTextueMode_AlphaOnly:
			{
				PRINTF_INFO("Texture mode: ALPHA-ONLY.\n");
				_atlasMapaBitsCustomView = new(this) AUMapaBits(_atlasMapaBits);
				BYTE* data = _atlasMapaBitsCustomView->datos();
				const MapaDeBitsDesciptor props = _atlasMapaBitsCustomView->propiedades(); NBASSERT(props.color == COLOR_RGBA8)
				SI32 y; const SI32 lines = props.alto; const SI32 cols = props.ancho;
				for(y = 0; y < lines; y++){
					BYTE* posAtLine = &data[(SI32)props.bytesPorLinea * y];
					const BYTE* posAtLineNext = posAtLine + ((props.bitsPorPixel / 8) * cols);
					while(posAtLine < posAtLineNext){
						posAtLine[0] = posAtLine[1] = posAtLine[2] = posAtLine[3];
						posAtLine[3] = 255;
						posAtLine += 4;
					}
				}
			}
			break;
		case ENEditorAtlasTextueMode_RedOnly:
			{
				PRINTF_INFO("Texture mode: RED-ONLY.\n");
				_atlasMapaBitsCustomView = new(this) AUMapaBits(_atlasMapaBits);
				BYTE* data = _atlasMapaBitsCustomView->datos();
				const MapaDeBitsDesciptor props = _atlasMapaBitsCustomView->propiedades(); NBASSERT(props.color == COLOR_RGBA8)
				SI32 y; const SI32 lines = props.alto; const SI32 cols = props.ancho;
				for(y = 0; y < lines; y++){
					BYTE* posAtLine = &data[(SI32)props.bytesPorLinea * y];
					const BYTE* posAtLineNext = posAtLine + ((props.bitsPorPixel / 8) * cols);
					while(posAtLine < posAtLineNext){
						posAtLine[1] = posAtLine[2] = 0;
						posAtLine[3] = 255;
						posAtLine += 4;
					}
				}
			}
			break;
		case ENEditorAtlasTextueMode_GreenOnly:
			{
				PRINTF_INFO("Texture mode: GREEN-ONLY.\n");
				_atlasMapaBitsCustomView = new(this) AUMapaBits(_atlasMapaBits);
				BYTE* data = _atlasMapaBitsCustomView->datos();
				const MapaDeBitsDesciptor props = _atlasMapaBitsCustomView->propiedades(); NBASSERT(props.color == COLOR_RGBA8)
				SI32 y; const SI32 lines = props.alto; const SI32 cols = props.ancho;
				for(y = 0; y < lines; y++){
					BYTE* posAtLine = &data[(SI32)props.bytesPorLinea * y];
					const BYTE* posAtLineNext = posAtLine + ((props.bitsPorPixel / 8) * cols);
					while(posAtLine < posAtLineNext){
						posAtLine[0] = posAtLine[2] = 0;
						posAtLine[3] = 255;
						posAtLine += 4;
					}
				}
			}
			break;
		case ENEditorAtlasTextueMode_BlueOnly:
			{
				PRINTF_INFO("Texture mode: BLUE-ONLY.\n");
				_atlasMapaBitsCustomView = new(this) AUMapaBits(_atlasMapaBits);
				BYTE* data = _atlasMapaBitsCustomView->datos();
				const MapaDeBitsDesciptor props = _atlasMapaBitsCustomView->propiedades(); NBASSERT(props.color == COLOR_RGBA8)
				SI32 y; const SI32 lines = props.alto; const SI32 cols = props.ancho;
				for(y = 0; y < lines; y++){
					BYTE* posAtLine = &data[(SI32)props.bytesPorLinea * y];
					const BYTE* posAtLineNext = posAtLine + ((props.bitsPorPixel / 8) * cols);
					while(posAtLine < posAtLineNext){
						posAtLine[0] = posAtLine[1] = 0;
						posAtLine[3] = 255;
						posAtLine += 4;
					}
				}
			}
			break;
		case ENEditorAtlasTextueMode_Gray:
		{
			PRINTF_INFO("Texture mode: GRAY.\n");
			_atlasMapaBitsCustomView = new(this) AUMapaBits(_atlasMapaBits);
			BYTE* data = _atlasMapaBitsCustomView->datos();
			const MapaDeBitsDesciptor props = _atlasMapaBitsCustomView->propiedades(); NBASSERT(props.color == COLOR_RGBA8)
			SI32 y; const SI32 lines = props.alto; const SI32 cols = props.ancho;
			for(y = 0; y < lines; y++){
				BYTE* posAtLine = &data[(SI32)props.bytesPorLinea * y];
				const BYTE* posAtLineNext = posAtLine + ((props.bitsPorPixel / 8) * cols);
				while(posAtLine < posAtLineNext){
					posAtLine[0] = posAtLine[1] = posAtLine[2] = (((SI32)posAtLine[0] + (SI32)posAtLine[1] + (SI32)posAtLine[2]) / 3);
					posAtLine[3] = 255;
					posAtLine += 4;
				}
			}
		}
			break;
		default:
			PRINTF_INFO("Texture mode: DEFAULT.\n");
			break;
	}
	//Set sprite texture
	_atlasTexture = NBGestorTexturas::texturaDesdeMapaBits((_atlasMapaBitsCustomView != NULL ? _atlasMapaBitsCustomView : _atlasMapaBits), "", ENTexturaModoPintado_Imagen_Precisa, ENTexturaMipMap_Inhabilitado, ENTexturaOrdenV_HaciaArriba, _atlasCoordMutliplier, 1.0f);
	if(_atlasTexture != NULL) _atlasTexture->retener();
	_atlasSprite->establecerTextura(_atlasTexture);
	PRINTF_INFO("Texture updated.\n");
}

void AUEditorAtlas::privTextureSwitchShowColor(){
	SI32 mode = _atlasTextureMode + 1; if(mode >= ENEditorAtlasTextueMode_Count) mode = 0;
	_atlasTextureMode = (ENEditorAtlasTextueMode)mode;
	PRINTF_INFO("Texture mode to %d.\n", mode);
	this->privAtlasRefreshTexture();
}

//

void AUEditorAtlas::puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after){
	_animObjetos->purgarAnimaciones();
	this->privOrganizarContenido();
}

void AUEditorAtlas::listImagesSelectionChanged(AUIListImages* obj, AUArregloNativoP<SI32>* selectionIndexes){
	if(obj == _guiImagesList){
		this->privUpdateAreasRepresentations(0);
	}
}

void AUEditorAtlas::listImagesNameChaged(AUIListImages* obj, const SI32 imageIndex, const char* pNewName){
	if(obj == _guiImagesList && _atlasMap != NULL){
		//Filter undesired characters
		const char* newName = pNewName; SI32 bytesIgnored = 0;
		AUCadenaMutable8* strFiltered = new(this) AUCadenaMutable8();
		while(*newName != '\0'){
			//45 = '-', 95 = '_', 48 ... 57 ('0' ... '9'), 65 ... 90 ('A' ... 'Z'), 97 ... 122 ('a' ... 'z')
			if(*newName == 45 || *newName == 95 || (*newName >= 48 && *newName <= 57) || (*newName >= 65 && *newName <= 90) || (*newName >= 97 && *newName <= 122)){
				strFiltered->agregar(*newName);
			} else {
				bytesIgnored++;
			}
			newName++;
		}
		_atlasMap->establecerAreaNombreEnIndice(imageIndex, strFiltered->str());
		//PRINTF_INFO("Area-name #%d changed to: '%s'.\n", (imageIndex + 1), strFiltered->str());
		if(bytesIgnored != 0){
			PRINTF_INFO("Area-name #%d filtered from '%s' to '%s'.\n", (imageIndex + 1), pNewName, strFiltered->str());
			obj->updateImageNameAtIndex(imageIndex, strFiltered->str());
		}
		strFiltered->liberar();
	}
}

bool AUEditorAtlas::menuOptionExecuteAction(AUIMenu* obj, const SI32 optionId){
	bool r = false;
	if(obj == _guiMenu && _atlasMap != NULL){
		switch (optionId) {
			case ENEditorAtlasButton_EditMesh:
				if(_guiEditorMesh == NULL){
					if(_atlasMapaBits != NULL){
						const AUArregloNativoP<SI32>* selIndexes = _guiImagesList->selectionIndexes();
						if(selIndexes->conteo == 1){
							const SI32 index = selIndexes->elem(0);
							const AUArregloNativoP<STAtlasArea>* areasArr = _atlasMap->areasOcupadas();
							if(index >=0 && index < areasArr->conteo){
								STAtlasArea* mapa	= areasArr->elemPtr(index);
								AUMapaBits* bitmap = new(this) AUMapaBits(_atlasMapaBits, mapa->area.x, mapa->area.y, mapa->area.ancho, mapa->area.alto);
								this->escenaQuitarDePrimerPlano();
								_guiEditorMesh = new(this) AUEditorMesh(_iEscena);
								_guiEditorMesh->setListener(this);
								_guiEditorMesh->openBitmap(bitmap, mapa->mallaMapa);
								_guiEditorMesh->escenaColocarEnPrimerPlano();
								bitmap->liberar();
								r = true;
							}
						} else if(selIndexes->conteo == 0){
							AUMallaMapa* meshMapToLoad = NULL;
							{
								AUMallaMapaMutable* meshMap = new(this) AUMallaMapaMutable(1);
								const AUCadenaLarga8* chunckAT = _atlasMapaBitsExtraChuncks->getChunckById("nbMH");
								if(chunckAT != NULL){
									AUArchivoEnBuffer* flujo = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer(&chunckAT->str()[4], chunckAT->tamano() - 4);
									flujo->lock();
									if(!AUMallaMapa::cargarXmlEn(meshMap, flujo)){
										PRINTF_ERROR("Could not load mesh from png data chunck: (%d bytes).\n", chunckAT->tamano());
									} else {
										PRINTF_INFO("Mesh loaded from png data chunck: (%d bytes).\n", chunckAT->tamano());
										meshMapToLoad = meshMap; meshMap->retener();
									}
									flujo->unlock();
									flujo->liberar();
								}
								meshMap->liberar();
							}
							this->escenaQuitarDePrimerPlano();
							_guiEditorMesh = new(this) AUEditorMesh(_iEscena);
							_guiEditorMesh->setListener(this);
							_guiEditorMesh->openBitmap(_atlasMapaBits, meshMapToLoad);
							_guiEditorMesh->escenaColocarEnPrimerPlano();
							if(meshMapToLoad != NULL) meshMapToLoad->liberar();
							r = true;
						}
					}
					//PRINTF_INFO("EDIT MESH.\n");
				}
				break;
			case ENEditorAtlasButton_Save:
				r = this->privAtlasSave();
				break;
			case ENEditorAtlasButton_ExportAll:
				r = this->privAtlasExportAll();
				break;
			case ENEditorAtlasButton_Refresh:
				if(_atlasMapaBits != NULL){
					this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_BITMAPS_FORCED);
					this->privAtlasRefreshTexture();
					r = true;
				}
				break;
			case ENEditorAtlasButton_PerfectPixelPrefered:
				{
					bool newPixPresPrefered = !_atlasProps->precisionPixelPrefererida();
					_atlasProps->establecerPrecisionPixelPreferida(newPixPresPrefered);
					_guiMenu->optionUpdate(ENEditorAtlasButton_PerfectPixelPrefered, newPixPresPrefered ? "PIXEL PRECISON ON" : "PIXEL PRECISON OFF");
					r = true;
				}
				break;
			case ENEditorAtlasButton_MaskPrefered:
				{
					bool newMaskPrefered = !_atlasProps->mascaraAlphaPreferido();
					r = true;
					if(newMaskPrefered){
						//Verify if it is mask compatible
						const STBitmapPixsDesc pxDesc = _atlasMapaBits->pixelsDesc();
						if((pxDesc.mask & NB_BITMAP_PIXS_DESC_BIT_RGB) != 0 || (pxDesc.mask & NB_BITMAP_PIXS_DESC_BIT_TRANSP) != 0){
							newMaskPrefered = false;
							r = false;
						}
					}
					_atlasProps->establecerMascaraAlphaPreferido(newMaskPrefered);
					_guiMenu->optionUpdate(ENEditorAtlasButton_MaskPrefered, newMaskPrefered ? "MASK MODE ON" : "MASK MODE OFF");
				}
				break;
			case ENEditorAtlasButton_TextureMode:
				this->privTextureSwitchShowColor();
				r = true;
				break;
			/*case ENEditorAtlasButton_InvertColor:
				{
					const AUArregloNativoP<SI32>* selIndexes = _guiImagesList->selectionIndexes();
					if(NBAtlasTools::invertColors(_atlasMapaBits, _atlasMap, selIndexes->conteo != 0 ? selIndexes : NULL)){
						this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_BITMAPS_FORCED);
						this->privAtlasRefreshTexture();
						r = true;
					}
				}
				break;*/
			/*case ENEditorAtlasButton_FixMasksBorders:
				{
					const AUArregloNativoP<SI32>* selIndexes = _guiImagesList->selectionIndexes();
					if(NBAtlasTools::fixMasksBorders(_atlasMapaBits, _atlasMap, 10, selIndexes->conteo != 0 ? selIndexes : NULL)){
						this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_BITMAPS_FORCED);
						this->privAtlasRefreshTexture();
						r = true;
					}
				}
				break;*/
			default:
				NBASSERT(false)
				break;
		}
	}
	return r;
	
}

void AUEditorAtlas::privOrganizarContenido(){
	const NBRectangulo visionPort = NBGestorEscena::puertoDeVision(_iEscena);
	//
	NBCajaAABB workspaceBox;
	workspaceBox.xMin = _workspaceCenter.x - (visionPort.ancho * 0.5f / _workspaceScale);
	workspaceBox.xMax = _workspaceCenter.x + (visionPort.ancho * 0.5f / _workspaceScale);
	workspaceBox.yMin = _workspaceCenter.y - (visionPort.alto * 0.5f / _workspaceScale);
	workspaceBox.yMax = _workspaceCenter.y + (visionPort.alto * 0.5f / _workspaceScale);
	//
	NBGestorEscena::establecerCajaProyeccion(_iEscena, workspaceBox.xMin, workspaceBox.xMax, workspaceBox.yMin, workspaceBox.yMax);
	//
	NBASSERT(_layerDrag->hijos()->conteo >= 2)
	if(_layerDrag->hijos()->conteo >= 2){
		AUEscenaObjeto* refObjTopRight = (AUEscenaObjeto*)_layerDrag->hijos()->elem(0);
		AUEscenaObjeto* refObjBttmLeft = (AUEscenaObjeto*)_layerDrag->hijos()->elem(1);
		refObjTopRight->establecerTraslacion(workspaceBox.xMin, workspaceBox.yMin);
		refObjBttmLeft->establecerTraslacion(workspaceBox.xMax, workspaceBox.yMax);
	}
	//Organize GUI
	{
		const NBCajaAABB cajaGUI	= NBGestorEscena::cajaProyeccionGrupo(_iEscena, ENGestorEscenaGrupo_GUI);
		const float widthGUI		= (cajaGUI.xMax - cajaGUI.xMin);
		const float imgListWidth	= widthGUI > (150.0 * 4.0f) ? 150.0f : (widthGUI / 4.0f);
		//ImageList
		{
			_guiImagesList->setLimitsSize(imgListWidth, cajaGUI.yMax - cajaGUI.yMin);
			_guiImagesList->establecerTraslacion(cajaGUI.xMin, cajaGUI.yMax);
		}
		//Menu
		{
			_guiMenu->establecerTraslacion(cajaGUI.xMin + imgListWidth + 8.0f, cajaGUI.yMax - 4.0f);
		}
		//Color-sample
		{
			const float marginH = NBGestorEscena::anchoPulgadasAEscena(_iEscena, 0.10f);
			const float marginV = NBGestorEscena::altoPulgadasAEscena(_iEscena, 0.10f);
			const NBCajaAABB sampleBox = _colorSample->cajaAABBLocal();
			_colorBorder->establecerTraslacion(cajaGUI.xMin + imgListWidth + marginH, cajaGUI.yMin + marginV);
			_colorSample->establecerTraslacion(cajaGUI.xMin + imgListWidth + marginH, cajaGUI.yMin + marginV);
			_colorTxt->establecerTraslacion(cajaGUI.xMin + imgListWidth + marginH + (sampleBox.xMax - sampleBox.xMin) + marginH, cajaGUI.yMin + marginV);
		}
	}
}

void AUEditorAtlas::privWorkspaceSetScale(const float workspaceScale){
	_workspaceScale = workspaceScale;
	if(_workspaceScale < 0.1f) _workspaceScale = 0.1f;
	if(_workspaceScale > 10.0f) _workspaceScale = 10.0f;
	//Show/hide rules
	SI32 visibilityBase;
	if(_workspaceScale >= 2.0f){
		visibilityBase = 1;
	} else if(_workspaceScale >= 1.0f){
		visibilityBase = 2;
	} else if(_workspaceScale >= 0.5f){
		visibilityBase = 4;
	} else {
		visibilityBase = 8;
	}
	SI32 i, j;
	for(i = 0; i < ENEditorAtlasRule_Count; i++){
		const SI32 linesCount = _atlasRules[i]->conteo;
		for(j = 0; j < linesCount; j++){
			AUEscenaFigura* fig = (AUEscenaFigura*)_atlasRules[i]->elem(j);
			fig->establecerVisible((j % visibilityBase) == 0);
		}
	}
	_atlasGridLayer->establecerVisible((_workspaceScale >= 4.0f));
	//
	this->privOrganizarContenido();
}

SI32 AUEditorAtlas::privIndexAreaWithObject(AUEscenaObjeto* obj){
	SI32 r = -1;
	SI32 i; const SI32 count = _atlasAreas->conteo;
	for(i = 0; i < count; i++){
		STEditorAtlasArea* area = _atlasAreas->elemPtr(i);
		if(obj == area->visualArea || obj == area->visualLimit[ENEditorAtlasRule_Left] || obj == area->visualLimit[ENEditorAtlasRule_Right] || obj == area->visualLimit[ENEditorAtlasRule_Top] || obj == area->visualLimit[ENEditorAtlasRule_Bottom]){
			r = i;
			break;
		}
	}
	return r;
}

SI32 AUEditorAtlas::privRemoveEmptyAreas(){
	SI32 removedCount = 0;
	const AUArregloNativoP<STAtlasArea>* areasArr = _atlasMap->areasOcupadas();
	SI32 i; const SI32 count = areasArr->conteo;
	for(i = (count - 1); i >= 0; i--){
		STAtlasArea* area = areasArr->elemPtr(i);
		if(area->area.ancho <= 0 || area->area.alto <= 0){
			_atlasMap->removerAreaEnIndice(i);
			_guiImagesList->removeImageAtIndex(i);
			removedCount++;
		}
	}
	return removedCount;
}

//

void AUEditorAtlas::privMovFill(){
	//Empty selection
	this->privMovClear(false);
	//Fill selection
	if(_atlasMapaBits != NULL){
		const AUArregloNativoP<SI32>* selArr =  _guiImagesList->selectionIndexes();
		if(selArr->conteo != 0){
			const NBTamano sizeHD	= _atlasTexture->tamanoHD();
			SI32 i; const SI32 conteo = selArr->conteo;
			for(i = 0; i < conteo; i++){
				const SI32 iArea = selArr->elem(i);
				const NBRectanguloUI16 areaMap = _atlasMap->areaEnIndice(iArea);
				STEditorAtlasAreaMov newMov;
				newMov.iArea				= iArea;
				newMov.originalBitMapArea	= areaMap;
				newMov.copyBitmap			= new(this) AUMapaBits(_atlasMapaBits, areaMap.x, areaMap.y, areaMap.ancho, areaMap.alto);
				newMov.copyTexture			= NBGestorTexturas::texturaDesdeMapaBits(newMov.copyBitmap, "", ENTexturaModoPintado_Imagen_Precisa, ENTexturaMipMap_Inhabilitado, ENTexturaOrdenV_HaciaArriba, _atlasCoordMutliplier, 1.0f);
				newMov.copySprite			= new(this) AUEscenaSprite(newMov.copyTexture, ENSpriteAlineaH_Der, ENSpriteAlineaV_Aba);
				newMov.copySprite->establecerTraslacion(areaMap.x, sizeHD.alto - areaMap.y);
				_atlasAreasSubLayer->agregarObjetoEscena(newMov.copySprite);
				_movAreaCopies.agregarElemento(newMov);
				//Format current position in texture (clear)
				_atlasMapaBits->formatearArea(areaMap.x, areaMap.y, areaMap.ancho, areaMap.alto, 0, 0, 0, 0);
			}
			//Reset mov dettas
			_movDeltaXAcum = _movDeltaYAcum = 0.0f;
			_movColisionDetected = false;
		}
	}
}

void AUEditorAtlas::privMovClear(const bool applyMovements){
	SI32 i; const SI32 conteo = _movAreaCopies.conteo;
	for(i = 0; i < conteo; i++){
		STEditorAtlasAreaMov* areMov = _movAreaCopies.elemPtr(i);
		if(!applyMovements){
			//Revert movement
			_atlasMap->establecerAreaEnIndice(areMov->iArea, areMov->originalBitMapArea.x, areMov->originalBitMapArea.y, areMov->originalBitMapArea.ancho, areMov->originalBitMapArea.alto);
			if(_atlasMapaBits != NULL){
				_atlasMapaBits->pegarMapaBitsRGBA(areMov->originalBitMapArea.x, areMov->originalBitMapArea.y, areMov->copyBitmap);
			}
		} else {
			//Set new location
			if(_atlasMapaBits != NULL){
				const NBRectanguloUI16 area = _atlasMap->areaEnIndice(areMov->iArea);
				_atlasMapaBits->pegarMapaBitsRGBA(area.x, area.y, areMov->copyBitmap);
			}
		}
		//Release objects
		AUEscenaContenedor* parent = (AUEscenaContenedor*)areMov->copySprite->contenedor();
		if(parent != NULL){ parent->quitarObjetoEscena(areMov->copySprite); }
		areMov->copySprite->liberar();	areMov->copySprite = NULL;
		areMov->copyTexture->liberar();	areMov->copyTexture = NULL;
		areMov->copyBitmap->liberar();	areMov->copyBitmap = NULL;
		areMov->iArea = -1;
	}
	_movAreaCopies.vaciar();
	_movDeltaXAcum = _movDeltaYAcum = 0.0f;
	_movColisionDetected = false;
}

void AUEditorAtlas::privMovApplyDelta(const float deltaX, const float deltaY){
	//Calculate real delta
	const float realDeltaX	= deltaX + _movDeltaXAcum;
	const float realDeltaY	= deltaY + _movDeltaYAcum;
	SI32 deltaXI			= ((SI32)realDeltaX / _atlasCoordMutliplier) * _atlasCoordMutliplier;
	SI32 deltaYI			= ((SI32)realDeltaY / _atlasCoordMutliplier) * _atlasCoordMutliplier;
	_movDeltaXAcum			= realDeltaX - (float)deltaXI;
	_movDeltaYAcum			= realDeltaY - (float)deltaYI;
	//
	if(_atlasMapaBits != NULL && _movAreaCopies.conteo != 0){
		const NBTamano sizeHD	= _atlasTexture->tamanoHD();
		SI32 i; const SI32 count = _movAreaCopies.conteo;
		//Calculate selection limits
		NBCajaAABBI selBox;
		{
			STEditorAtlasAreaMov* areMov = _movAreaCopies.elemPtr(0);
			const NBRectanguloUI16 areaMap = _atlasMap->areaEnIndice(areMov->iArea);
			NBCAJAAABB_INICIALIZAR_CON_PUNTO(selBox, areaMap.x, areaMap.y)
			NBCAJAAABB_ENVOLVER_PUNTO_SIGUIENTE(selBox, areaMap.x + areaMap.ancho, areaMap.y + areaMap.alto)
			for(i = 1; i < count; i++){
				STEditorAtlasAreaMov* areMov = _movAreaCopies.elemPtr(i);
				const NBRectanguloUI16 areaMap = _atlasMap->areaEnIndice(areMov->iArea);
				NBCAJAAABB_ENVOLVER_PUNTO_SIGUIENTE(selBox, areaMap.x, areaMap.y)
				NBCAJAAABB_ENVOLVER_PUNTO_SIGUIENTE(selBox, areaMap.x + areaMap.ancho, areaMap.y + areaMap.alto)
			}
		}
		//Validate size of movement
		if(selBox.xMin + deltaXI < 0.0f){
			deltaXI = - selBox.xMin;
		} else if(selBox.xMax + deltaXI > sizeHD.ancho){
			deltaXI = sizeHD.ancho - selBox.xMax;
		}
		if(selBox.yMin + deltaYI < 0.0f){
			deltaYI = - selBox.yMin;
		} else if(selBox.yMax + deltaYI > sizeHD.alto){
			deltaYI = sizeHD.alto - selBox.yMax;
		}
		//Move selection
		for(i = 0; i < count; i++){
			STEditorAtlasAreaMov* areMov = _movAreaCopies.elemPtr(i);
			const NBRectanguloUI16 areaMap = _atlasMap->areaEnIndice(areMov->iArea);
			//Move copy
			const NBPunto pos = areMov->copySprite->traslacion();
			areMov->copySprite->establecerTraslacion(pos.x + deltaXI, pos.y - deltaYI); //y-inverted in escene
			//Move area
			_atlasMap->establecerAreaEnIndice(areMov->iArea, areaMap.x + deltaXI, areaMap.y + deltaYI, areaMap.ancho, areaMap.alto);
		}
		_movColisionDetected = this->privMovAreaColides();
		this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_SHAPES); //Note: do not update bitmaps
	}
}

bool AUEditorAtlas::privMovAreaColides(){
	bool r = false;
	const AUArregloNativoP<STAtlasArea>* areas = _atlasMap->areasOcupadas();
	const SI32 countAreas = areas->conteo;
	SI32 i, i2, i3; const SI32 countMov = _movAreaCopies.conteo;
	for(i = 0; i < countMov && !r; i++){
		STEditorAtlasAreaMov* areMov = _movAreaCopies.elemPtr(i);
		const NBRectanguloUI16 areaMap = _atlasMap->areaEnIndice(areMov->iArea);
		//Compare wiht other areas
		for(i2 = 0; i2 < countAreas; i2++){
			if(i2 != areMov->iArea){
				bool isAtMovement = false;
				for(i3 = 0; i3 < countMov; i3++){
					STEditorAtlasAreaMov* areMov = _movAreaCopies.elemPtr(i3);
					if(areMov->iArea == i2){
						isAtMovement = true;
						break;
					}
				}
				if(!isAtMovement){
					const NBRectanguloUI16 otherAreaMap = _atlasMap->areaEnIndice(i2);
					if(!(areaMap.x >= (otherAreaMap.x + otherAreaMap.ancho) || (areaMap.x + areaMap.ancho) <= otherAreaMap.x || areaMap.y >= (otherAreaMap.y + otherAreaMap.alto) || (areaMap.y + areaMap.alto) <= otherAreaMap.y)){
						r = true;
						break;
					}
				}
			}
		}
	}
	return r;
}

//

void AUEditorAtlas::privUpdateAreasRepresentations(const SI32 updateMask){
	if(_atlasTexture != NULL){
		bool updateShapes		= ((updateMask & EDITOR_ATLAS_UPDATE_MASK_SHAPES) != 0);
		bool updateBitmaps		= ((updateMask & (EDITOR_ATLAS_UPDATE_MASK_BITMAPS_IF_NECESARY | EDITOR_ATLAS_UPDATE_MASK_BITMAPS_FORCED)) != 0);
		bool updateBitmapsForced= ((updateMask & EDITOR_ATLAS_UPDATE_MASK_BITMAPS_FORCED) != 0);
		const NBTamano sizeHD	= _atlasTexture->tamanoHD();
		//Add new areas (if necesary)
		if(_atlasMap->areasOcupadas()->conteo > _atlasAreas->conteo){
			const AUArregloNativoP<STAtlasArea>* areasMap = _atlasMap->areasOcupadas();
			SI32 i, j; const SI32 conteo = areasMap->conteo;
			for(i = _atlasAreas->conteo; i < conteo; i++){
				STEditorAtlasArea area; AUEscenaFigura* fig;
				STAtlasArea* areaMap = areasMap->elemPtr(i);
				//Add first limits references
				for(j = 0; j < ENEditorAtlasRule_Count; j++){
					fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
					fig->agregarCoordenadaLocal(0.0f, 0.0f);
					fig->agregarCoordenadaLocal(0.0f, 0.0f);
					fig->establecerEscuchadorTouches(this, this);
					fig->establecerMultiplicadorColor8(0, 255, 0, 255);
					//_atlasAreasLayer->agregarObjetoEscena(fig);
					area.visualLimit[j] = fig;
					area.visualLimitHoverCount[j] = 0;
				}
				//Add last the area reference (let it be at top)
				fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_PoligonoCerrado);
				fig->establecerModo(ENEscenaFiguraModo_Borde);
				fig->agregarCoordenadaLocal(0.0f, 0.0f);
				fig->agregarCoordenadaLocal(0.0f, 0.0f);
				fig->agregarCoordenadaLocal(0.0f, 0.0f);
				fig->agregarCoordenadaLocal(0.0f, 0.0f);
				_atlasAreasLayer->agregarObjetoEscena(fig);
				area.visualArea = fig;
				//
				area.bitMap	= NULL;
				//
				if(areaMap->nombre == NULL){
					_guiImagesList->addImageAtIndex(i, area.bitMap, "", _atlasCoordMutliplier, 1.0f);
					PRINTF_INFO("New-area with no-name.\n");
				} else {
					_guiImagesList->addImageAtIndex(i, area.bitMap, areaMap->nombre->str(), _atlasCoordMutliplier, 1.0f);
					PRINTF_INFO("New-area with name: '%s'.\n", areaMap->nombre->str());
				}
				_atlasAreas->agregarElemento(area);
			}
			updateShapes = true;
		}
		//Remove extra areas (if necesary)
		NBASSERT(_guiImagesList->imageCount() == _atlasMap->areasOcupadas()->conteo)
		if(_atlasAreas->conteo > _atlasMap->areasOcupadas()->conteo){
			SI32 i, j; const SI32 conteo = _atlasMap->areasOcupadas()->conteo;
			for(i = _atlasAreas->conteo - 1; i >= conteo; i--){
				STEditorAtlasArea* area = _atlasAreas->elemPtr(i);
				AUEscenaContenedor* parent = (AUEscenaContenedor*)area->visualArea->contenedor();
				if(parent != NULL) { parent->quitarObjetoEscena(area->visualArea); }
				area->visualArea->liberar();
				for(j = 0; j < ENEditorAtlasRule_Count; j++){
					AUEscenaContenedor* parent = (AUEscenaContenedor*)area->visualLimit[j]->contenedor();
					if(parent != NULL) { parent->quitarObjetoEscena(area->visualLimit[j]); }
					area->visualLimit[j]->liberar();
				}
				if(area->bitMap != NULL) area->bitMap->liberar();
				_atlasAreas->quitarElementoEnIndice(i);
			}
			updateShapes = true;
		}
		//Update references visual state
		AUCadenaMutable8* strTmp = NULL;
		const AUArregloNativoP<SI32>* selIndexes = _guiImagesList->selectionIndexes();
		const bool multipleSelection = (selIndexes->conteo > 1);
		const AUArregloNativoP<STAtlasArea>* areasArr = _atlasMap->areasOcupadas();
		SI32 i; const SI32 conteo = areasArr->conteo;
		for(i = 0; i < conteo; i++){
			AUEscenaFigura* fig;
			SI8 hoverCount;
			STAtlasArea* mapa		= areasArr->elemPtr(i);
			STEditorAtlasArea* area = _atlasAreas->elemPtr(i);
			const bool isSelected	= (selIndexes->indiceDe(i) != -1);
			//
			fig = area->visualArea;
			if(_movColisionDetected){
				fig->establecerMultiplicadorColor8(255, 150, 150, 255);
			} else if(isSelected){
				fig->establecerMultiplicadorColor8(150, 150, 255, 255);
			} else {
				fig->establecerMultiplicadorColor8(255, 255, 255, 255);
			}
			fig->moverVerticeHacia(0, 0.0f, 0.0f);
			fig->moverVerticeHacia(1, mapa->area.ancho, 0.0f);
			fig->moverVerticeHacia(2, mapa->area.ancho, mapa->area.alto);
			fig->moverVerticeHacia(3, 0.0f, mapa->area.alto);
			fig->establecerTraslacion(mapa->area.x, sizeHD.alto - mapa->area.y - mapa->area.alto);
			if(strTmp == NULL){
				strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
			}
			strTmp->vaciar();
			strTmp->agregarConFormato("(%d, %d)-(%d, %d)", (SI32)mapa->area.x, (SI32)mapa->area.y, (SI32)mapa->area.ancho, (SI32)mapa->area.alto);
			_guiImagesList->updateImageExtraDetailsAtIndex(i, strTmp->str());
			//
			hoverCount = area->visualLimitHoverCount[ENEditorAtlasRule_Left];
			fig = area->visualLimit[ENEditorAtlasRule_Left];
			fig->moverVerticeHacia(0, 0.0f, 0.0f);
			fig->moverVerticeHacia(1, 0.0f, sizeHD.alto);
			fig->establecerTraslacion(mapa->area.x, 0.0f);
			fig->establecerMultiplicadorColor8(hoverCount > 0 ? 255 : 0, 0, hoverCount > 0 ? 0 : 255, 255);
			if(isSelected && !multipleSelection){
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != _atlasAreasLayer) _atlasAreasLayer->agregarObjetoEscena(fig);
			} else {
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != NULL) parent->quitarObjetoEscena(fig);
			}
			//
			hoverCount = area->visualLimitHoverCount[ENEditorAtlasRule_Right];
			fig = area->visualLimit[ENEditorAtlasRule_Right];
			fig->moverVerticeHacia(0, 0.0f, 0.0f);
			fig->moverVerticeHacia(1, 0.0f, sizeHD.alto);
			fig->establecerTraslacion(mapa->area.x + mapa->area.ancho, 0.0f);
			fig->establecerMultiplicadorColor8(hoverCount > 0 ? 255 : 0, 0, hoverCount > 0 ? 0 : 255, 255);
			if(isSelected && !multipleSelection){
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != _atlasAreasLayer) _atlasAreasLayer->agregarObjetoEscena(fig);
			} else {
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != NULL) parent->quitarObjetoEscena(fig);
			}
			//
			hoverCount = area->visualLimitHoverCount[ENEditorAtlasRule_Top];
			fig = area->visualLimit[ENEditorAtlasRule_Top];
			fig->moverVerticeHacia(0, 0.0f, 0.0f);
			fig->moverVerticeHacia(1, sizeHD.ancho, 0.0f);
			fig->establecerTraslacion(0.0f, sizeHD.alto - mapa->area.y);
			fig->establecerMultiplicadorColor8(hoverCount > 0 ? 255 : 0, 0, hoverCount > 0 ? 0 : 255, 255);
			if(isSelected && !multipleSelection){
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != _atlasAreasLayer) _atlasAreasLayer->agregarObjetoEscena(fig);
			} else {
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != NULL) parent->quitarObjetoEscena(fig);
			}
			//
			hoverCount = area->visualLimitHoverCount[ENEditorAtlasRule_Bottom];
			fig = area->visualLimit[ENEditorAtlasRule_Bottom];
			fig->moverVerticeHacia(0, 0.0f, 0.0f);
			fig->moverVerticeHacia(1, sizeHD.ancho, 0.0f);
			fig->establecerTraslacion(0.0f, sizeHD.alto - mapa->area.y - mapa->area.alto);
			fig->establecerMultiplicadorColor8(hoverCount > 0 ? 255 : 0, 0, hoverCount > 0 ? 0 : 255, 255);
			if(isSelected && !multipleSelection){
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != _atlasAreasLayer) _atlasAreasLayer->agregarObjetoEscena(fig);
			} else {
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != NULL) parent->quitarObjetoEscena(fig);
			}
			//
			if(updateBitmaps){
				if(updateBitmapsForced || area->bitMap == NULL || area->bitMapArea.x != mapa->area.x || area->bitMapArea.y != mapa->area.y || area->bitMapArea.ancho != mapa->area.ancho || area->bitMapArea.alto != mapa->area.alto){
					if(area->bitMap != NULL){
						area->bitMap->liberar();
						area->bitMap = NULL;
					}
					if(mapa->area.ancho != 0 && mapa->area.alto != 0){
						area->bitMap		= new(this) AUMapaBits(_atlasMapaBits, mapa->area.x, mapa->area.y, mapa->area.ancho, mapa->area.alto);
						area->bitMapArea	= mapa->area;
						_guiImagesList->updateImageAtIndex(i, area->bitMap, _atlasCoordMutliplier, 1.0f);
					}
				}
			}
		}
		//
		if(strTmp != NULL){
			strTmp->liberar();
		}
	}
}

//

bool AUEditorAtlas::editorMeshCloseCommand(AUEditorMesh* obj){
	bool r = false;
	if(_guiEditorMesh != NULL && obj == _guiEditorMesh){
		_guiEditorMesh->escenaQuitarDePrimerPlano();
		this->escenaColocarEnPrimerPlano();
		_guiEditorMesh->autoLiberar();
		_guiEditorMesh = NULL;
		r = true;
	}
	return r;
}

bool AUEditorAtlas::editorMeshSaveCommand(AUEditorMesh* obj){
	bool r = false;
	if(_guiEditorMesh != NULL && obj == _guiEditorMesh){
		AUMapaBits* bitmap = _guiEditorMesh->meshBitmap();
		AUMallaMapa* meshMap = _guiEditorMesh->meshMap();
		if(bitmap != NULL && meshMap != NULL){
			const AUArregloNativoP<SI32>* selIndexes = _guiImagesList->selectionIndexes();
			if(selIndexes->conteo == 1){
				//Save selected area meshMap
				const SI32 index = selIndexes->elem(0);
				const AUArregloNativoP<STAtlasArea>* areasArr = _atlasMap->areasOcupadas();
				if(index >=0 && index < areasArr->conteo){
					STAtlasArea* mapa	= areasArr->elemPtr(index);
					const NBTamanoI tamBitmap	= bitmap->tamano();
					if(mapa->area.ancho == tamBitmap.ancho && mapa->area.alto == tamBitmap.alto){
						_atlasMap->establecerAreaMallaEnIndice(index, meshMap);
						r = true;
					}
				}
			} else if(selIndexes->conteo == 0){
				//Save global meshMap
				AUArchivoEnBuffer* flujo = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer();
				flujo->lock();
				if(!AUMallaMapa::guardarEnXml(meshMap, flujo)){
					PRINTF_ERROR("Could not generate xml of mesh.\n");
				} else {
					const AUCadena8* buffStream = flujo->buffer();
					//Save png-chunck
					_atlasMapaBitsExtraChuncks->setChunck("nbMH", (const BYTE*)buffStream->str(), buffStream->tamano());
					//Save mesh backup file
					_guiEditorMesh->saveMeshBackupFile(meshMap, _atlasFilePath->str());
					//Save png
					this->privAtlasSave();
					//
					r =true;
				}
				flujo->unlock();
				flujo->liberar();
			}
		}
	}
	return r;
}

//

void AUEditorAtlas::textboxFocoObtenido(AUITextBox* obj){
	//
}

void AUEditorAtlas::textboxFocoPerdido(AUITextBox* obj, const ENNBKeybFocusLostType actionType){
	//
}

bool AUEditorAtlas::textboxMantenerFocoAnteConsumidorDeTouch(AUEscenaObjeto* consumidorTouch){
	return false;
}

void AUEditorAtlas::textboxContenidoModificado(AUITextBox* obj, const char* strContenido){
	//
}

//

void AUEditorAtlas::tickAnimacion(float segsTranscurridos){
	//
}

//TOUCHES
void AUEditorAtlas::touchIniciado(STGTouch* touch, const NBPunto &posTouchEscena, AUEscenaObjeto* objeto){
	if(_atlasTexture != NULL){
		if(objeto == _layerDrag){
			const NBTamano sizeHD = _atlasTexture->tamanoHD();
			NBPuntoI posInitialM;
			posInitialM.x = ((SI32)posTouchEscena.x / _atlasCoordMutliplier) * _atlasCoordMutliplier;
			posInitialM.y = ((SI32)posTouchEscena.y / _atlasCoordMutliplier) * _atlasCoordMutliplier;
			if(posInitialM.x >= 0 && posInitialM.y >= 0 && posInitialM.x < sizeHD.ancho && posInitialM.y < sizeHD.alto){
				SI32 iFirstAreaOver = -1;
				STAtlasArea* firstAreaOver = NULL;
				const AUArregloNativoP<STAtlasArea>* areasArr = _atlasMap->areasOcupadas();
				SI32 i; const SI32 conteo = areasArr->conteo;
				for(i = 0; i < conteo; i++){
					STAtlasArea* mapa = areasArr->elemPtr(i);
					if(posInitialM.x >= mapa->area.x && posInitialM.y >= (sizeHD.alto - mapa->area.y - mapa->area.alto) && posInitialM.x < (mapa->area.x + mapa->area.ancho) && posInitialM.y < (sizeHD.alto - mapa->area.y)){
						iFirstAreaOver = i;
						firstAreaOver = mapa;
						break;
					}
				}
				if(firstAreaOver == NULL){
					if(_newAreaTouch == NULL){
						_atlasMap->agregarAreaOcupada(posInitialM.x, sizeHD.alto - posInitialM.y, 0, 0, 0, NULL, NULL);
						_newAreaDragH	= ENEditorAtlasDragH_Undef;
						_newAreaDragV	= ENEditorAtlasDragV_Undef;
						_newAreaTouch	= touch;
						_guiImagesList->selectionClear();
						this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_SHAPES);
						_guiImagesList->selectionAddIndex(_atlasMap->areasOcupadas()->conteo - 1);
						this->privUpdateAreasRepresentations(0);
					}
				} else {
					if(_selAreaIndex == -1){
						_selAreaIndex	= iFirstAreaOver;
						_selAreaTouch	= touch;
						_selAreaMoving	= false;
					}
				}
			}
			NBGestorTeclas::escuchadorEstablecer(this, this);
		}
	}
}

void AUEditorAtlas::touchMovido(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	if(_atlasTexture != NULL){
		NBPuntoI posInitialM, posBeforeM, posCurrentM;
		posInitialM.x = ((SI32)posInicialEscena.x / _atlasCoordMutliplier) * _atlasCoordMutliplier;
		posInitialM.y = ((SI32)posInicialEscena.y / _atlasCoordMutliplier) * _atlasCoordMutliplier;
		//
		posBeforeM.x = ((SI32)posAnteriorEscena.x / _atlasCoordMutliplier) * _atlasCoordMutliplier;
		posBeforeM.y = ((SI32)posAnteriorEscena.y / _atlasCoordMutliplier) * _atlasCoordMutliplier;
		//
		posCurrentM.x = ((SI32)posActualEscena.x / _atlasCoordMutliplier) * _atlasCoordMutliplier;
		posCurrentM.y = ((SI32)posActualEscena.y / _atlasCoordMutliplier) * _atlasCoordMutliplier;
		//
		if(objeto == _layerDrag){
			if(touch != _newAreaTouch){
				//Determine if selection is tranformed to a drag
				if(touch == _selAreaTouch){
					if(!_selAreaMoving){
						const float dragDist = NBPUNTO_DISTANCIA(posInicialEscena.x, posInicialEscena.y, posActualEscena.x, posActualEscena.y);
						if(dragDist > 4.0f){
							_selAreaMoving = true;
							this->privMovFill();
							this->privAtlasRefreshTexture();
						}
					}
					if(_selAreaMoving){
						this->privMovApplyDelta((posActualEscena.x - posAnteriorEscena.x), (posAnteriorEscena.y - posActualEscena.y)); // y-inverted in bitmap
					}
				}
				//Execute drag
				if(touch != _selAreaTouch){
					_workspaceCenter.x -= (posActualEscena.x - posAnteriorEscena.x);
					_workspaceCenter.y -= (posActualEscena.y - posAnteriorEscena.y);
					this->privOrganizarContenido();
				}
			} else {
				//New area creation
				const AUArregloNativoP<STAtlasArea>* areasArr = _atlasMap->areasOcupadas();
				const SI32 iNewArea = areasArr->conteo - 1;
				NBASSERT(iNewArea >= 0)
				if(iNewArea >= 0){
					const NBTamano sizeHD	= _atlasTexture->tamanoHD();
					//Fix current position (to texture limits)
					NBPuntoI curPosFixed	= posCurrentM;
					if(curPosFixed.x < 0.0f){ curPosFixed.x = 0.0f; }
					if(curPosFixed.x > sizeHD.ancho){ curPosFixed.x = sizeHD.ancho; }
					if(curPosFixed.y < 0.0f){ curPosFixed.y = 0.0f; }
					if(curPosFixed.y > sizeHD.alto){ curPosFixed.y = sizeHD.alto; }
					//Set drag direction
					if(_newAreaDragH == ENEditorAtlasDragH_Undef){
						if(posInitialM.x < posCurrentM.x){
							_newAreaDragH = ENEditorAtlasDragH_Right;
						} else if(posInitialM.x > posCurrentM.x){
							_newAreaDragH = ENEditorAtlasDragH_Left;
						}
					}
					if(_newAreaDragV == ENEditorAtlasDragV_Undef){
						if(posInitialM.y < posCurrentM.y){
							_newAreaDragV = ENEditorAtlasDragV_Up;
						} else if(posInitialM.y > posCurrentM.y){
							_newAreaDragV = ENEditorAtlasDragV_Down;
						}
					}
					//Fix current position (to drag's direction)
					if(_newAreaDragH == ENEditorAtlasDragH_Right){
						if(curPosFixed.x < posInitialM.x) { curPosFixed.x = posInitialM.x; }
					} else if(_newAreaDragH == ENEditorAtlasDragH_Left){
						if(curPosFixed.x > posInitialM.x) { curPosFixed.x = posInitialM.x; }
					}
					if(_newAreaDragV == ENEditorAtlasDragV_Up){
						if(curPosFixed.y < posInitialM.y) { curPosFixed.y = posInitialM.y; }
					} else if(_newAreaDragV == ENEditorAtlasDragV_Down){
						if(curPosFixed.y > posInitialM.y) { curPosFixed.y = posInitialM.y; }
					}
					//Fix current position (avoid colision with other areas)
					const float curLimitUp		= (posInitialM.y < curPosFixed.y ? curPosFixed.y : posInitialM.y);
					const float curLimitDown	= (posInitialM.y < curPosFixed.y ? posInitialM.y : curPosFixed.y);
					if(_newAreaDragH == ENEditorAtlasDragH_Right){
						SI32 i;
						for(i = 0; i < iNewArea; i++){
							STAtlasArea* mapa = areasArr->elemPtr(i);
							if(curPosFixed.x >= mapa->area.x && mapa->area.x > posInitialM.x && !(curLimitUp < (sizeHD.alto - mapa->area.y - mapa->area.alto) || curLimitDown > (sizeHD.alto - mapa->area.y))){
								curPosFixed.x = mapa->area.x - 1.0f;
							}
						}
					} else if(_newAreaDragH == ENEditorAtlasDragH_Left){
						SI32 i; const float curLimitDown = posInitialM.y;
						for(i = 0; i < iNewArea; i++){
							STAtlasArea* mapa = areasArr->elemPtr(i);
							if(curPosFixed.x < (mapa->area.x + mapa->area.ancho) && (mapa->area.x + mapa->area.ancho) < posInitialM.x && !(curLimitUp < (sizeHD.alto - mapa->area.y - mapa->area.alto) || curLimitDown > (sizeHD.alto - mapa->area.y))){
								curPosFixed.x = mapa->area.x + mapa->area.ancho;
							}
						}
					}
					const float curLimitLeft	= (posInitialM.x < curPosFixed.x ? posInitialM.x: curPosFixed.x);
					const float curLimitRight	= (posInitialM.x < curPosFixed.x ? curPosFixed.x : posInitialM.x);
					if(_newAreaDragV == ENEditorAtlasDragV_Up){
						SI32 i;
						for(i = 0; i < iNewArea; i++){
							STAtlasArea* mapa = areasArr->elemPtr(i);
							if(curPosFixed.y >= (sizeHD.alto - mapa->area.y - mapa->area.alto) && (sizeHD.alto - mapa->area.y - mapa->area.alto) > posInitialM.y && !(curLimitRight < mapa->area.x || curLimitLeft > (mapa->area.x + mapa->area.ancho))){
								curPosFixed.y = (sizeHD.alto - mapa->area.y - mapa->area.alto) - 1.0f;
							}
						}
					} else if(_newAreaDragV == ENEditorAtlasDragV_Down){
						SI32 i;
						for(i = 0; i < iNewArea; i++){
							STAtlasArea* mapa = areasArr->elemPtr(i);
							if(curPosFixed.y < (sizeHD.alto - mapa->area.y) && (sizeHD.alto - mapa->area.y) < posInitialM.y && !(curLimitRight < mapa->area.x || curLimitLeft > (mapa->area.x + mapa->area.ancho))){
								curPosFixed.y = (sizeHD.alto - mapa->area.y);
							}
						}
					}
					//
					NBRectangulo areaSel;
					if(posInitialM.x < curPosFixed.x){
						areaSel.x		= posInitialM.x;
						areaSel.ancho	= (curPosFixed.x - posInitialM.x);
					} else {
						areaSel.x		= curPosFixed.x;
						areaSel.ancho	= (posInitialM.x - curPosFixed.x);
					}
					if(posInitialM.y < curPosFixed.y){
						areaSel.y		= sizeHD.alto - curPosFixed.y;
						areaSel.alto	= (curPosFixed.y - posInitialM.y);
					} else {
						areaSel.y		= sizeHD.alto - posInitialM.y;
						areaSel.alto	= (posInitialM.y - curPosFixed.y);
					}
					_atlasMap->establecerAreaEnIndice(iNewArea, areaSel.x, areaSel.y, areaSel.ancho, areaSel.alto);
					this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_SHAPES);
				}
			}
		} else {
			const SI32 iAreaDrag = privIndexAreaWithObject(objeto);
			if(iAreaDrag != -1){
				SI32 limitsUpdated			= 0;
				const AUArregloNativoP<STAtlasArea>* areasArr = _atlasMap->areasOcupadas();
				const SI32 areasArrCount	= areasArr->conteo;
				STEditorAtlasArea* area		= _atlasAreas->elemPtr(iAreaDrag);
				NBRectanguloUI16 areaMap	= areasArr->elemPtr(iAreaDrag)->area;
				NBPuntoI curPosFixed		= posCurrentM;
				const NBTamano sizeHD		= _atlasTexture->tamanoHD();
				if(objeto == area->visualLimit[ENEditorAtlasRule_Left]){
					if(curPosFixed.x < 0.0f){
						curPosFixed.x = 0.0f;
					} else if(curPosFixed.x > (areaMap.x + areaMap.ancho)){
						curPosFixed.x = (areaMap.x + areaMap.ancho);
					}
					SI32 i;
					for(i = 0; i < areasArrCount; i++){
						if(i != iAreaDrag){
							const NBRectanguloUI16 otherAreaMap = areasArr->elemPtr(i)->area;
							if(curPosFixed.x < (otherAreaMap.x + otherAreaMap.ancho) && areaMap.x >= (otherAreaMap.x + otherAreaMap.ancho) && !(areaMap.y > (otherAreaMap.y + otherAreaMap.alto) || (areaMap.y + areaMap.alto) < otherAreaMap.y)){
								curPosFixed.x = (otherAreaMap.x + otherAreaMap.ancho);
							}
						}
					}
					NBASSERT(curPosFixed.x <= (areaMap.x + areaMap.ancho))
					const SI16 delta = curPosFixed.x - areaMap.x;
					if(delta != 0){
						areaMap.x += delta;
						areaMap.ancho -= delta;
						limitsUpdated++;
					}
				} else if(objeto == area->visualLimit[ENEditorAtlasRule_Right]){
					if(curPosFixed.x > sizeHD.ancho){
						curPosFixed.x = sizeHD.ancho;
					} else if(curPosFixed.x < areaMap.x){
						curPosFixed.x = areaMap.x;
					}
					SI32 i;
					for(i = 0; i < areasArrCount; i++){
						if(i != iAreaDrag){
							const NBRectanguloUI16 otherAreaMap = areasArr->elemPtr(i)->area;
							if(curPosFixed.x > otherAreaMap.x && (areaMap.x + areaMap.ancho) <= otherAreaMap.x  && !(areaMap.y > (otherAreaMap.y + otherAreaMap.alto) || (areaMap.y + areaMap.alto) < otherAreaMap.y)){
								curPosFixed.x = otherAreaMap.x;
							}
						}
					}
					NBASSERT(curPosFixed.x >= areaMap.x)
					const SI16 delta = curPosFixed.x - (areaMap.x + areaMap.ancho);
					if(delta != 0){
						areaMap.ancho += delta;
						limitsUpdated++;
					}
				} else if(objeto == area->visualLimit[ENEditorAtlasRule_Top]){
					if(curPosFixed.y > sizeHD.alto){
						curPosFixed.y = sizeHD.alto;
					} else if(curPosFixed.y < (sizeHD.alto - areaMap.y - areaMap.alto)){
						curPosFixed.y = (sizeHD.alto - areaMap.y - areaMap.alto);
					}
					SI32 i;
					for(i = 0; i < areasArrCount; i++){
						if(i != iAreaDrag){
							const NBRectanguloUI16 otherAreaMap = areasArr->elemPtr(i)->area;
							if(curPosFixed.y > (sizeHD.alto - otherAreaMap.y - otherAreaMap.alto) && areaMap.y >= (otherAreaMap.y + otherAreaMap.alto) && !(areaMap.x > (otherAreaMap.x + otherAreaMap.ancho) || (areaMap.x + areaMap.ancho) < otherAreaMap.x)){
								curPosFixed.y = (sizeHD.alto - otherAreaMap.y - otherAreaMap.alto);
							}
						}
					}
					NBASSERT(curPosFixed.y >= (sizeHD.alto - areaMap.y - areaMap.alto))
					const SI16 delta = (sizeHD.alto - areaMap.y) - curPosFixed.y;
					if(delta != 0){
						areaMap.y += delta;
						areaMap.alto -= delta;
						limitsUpdated++;
					}
				} else if(objeto == area->visualLimit[ENEditorAtlasRule_Bottom]){
					if(curPosFixed.y < 0.0f){
						curPosFixed.y = 0.0f;
					} else if(curPosFixed.y > (sizeHD.alto - areaMap.y)){
						curPosFixed.y = (sizeHD.alto - areaMap.y);
					}
					SI32 i;
					for(i = 0; i < areasArrCount; i++){
						if(i != iAreaDrag){
							const NBRectanguloUI16 otherAreaMap = areasArr->elemPtr(i)->area;
							if(curPosFixed.y < (sizeHD.alto - otherAreaMap.y) && (areaMap.y + areaMap.alto) <= otherAreaMap.y && !(areaMap.x > (otherAreaMap.x + otherAreaMap.ancho) || (areaMap.x + areaMap.ancho) < otherAreaMap.x)){
								curPosFixed.y = (sizeHD.alto - otherAreaMap.y);
							}
						}
					}
					NBASSERT(curPosFixed.y <= (sizeHD.alto - areaMap.y))
					const SI16 delta = (sizeHD.alto - areaMap.y - areaMap.alto) - curPosFixed.y;
					if(delta != 0){
						areaMap.alto += delta;
						limitsUpdated++;
					}
				}
				if(limitsUpdated != 0){
					_atlasMap->establecerAreaEnIndice(iAreaDrag, areaMap.x, areaMap.y, areaMap.ancho, areaMap.alto);
					this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_SHAPES);
				}
			}
		}
	}
}

void AUEditorAtlas::touchFinalizado(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	const NBCajaAABB cajaEscena = objeto->cajaAABBEnEscena();
	if(NBCAJAAABB_INTERSECTA_PUNTO(cajaEscena, posActualEscena.x, posActualEscena.y)){
		//
	}
	//Finalize area selection
	if(_selAreaTouch == touch){
		if(_selAreaMoving){
			this->privMovClear(!_movColisionDetected);
			this->privAtlasRefreshTexture();
		} else if(_selAreaIndex >= 0){
			_guiImagesList->selectionSetIndex(_selAreaIndex);
		}
		_selAreaIndex	= -1;
		_selAreaTouch	= NULL;
		_selAreaMoving	= false;
		this->privUpdateAreasRepresentations(0);
	}
	//Finalize new area creation
	if(_newAreaTouch == touch){
		_newAreaTouch = NULL;
		_guiImagesList->selectionSetIndex(_atlasAreas->conteo - 1);
		this->privRemoveEmptyAreas();
		this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_SHAPES | EDITOR_ATLAS_UPDATE_MASK_BITMAPS_IF_NECESARY);
	}
	//Finalize limit-drag (remove empty ranges)
	if(objeto != _layerDrag){
		const SI32 iAreaDrag = privIndexAreaWithObject(objeto);
		if(iAreaDrag != -1){
			STEditorAtlasArea* area	= _atlasAreas->elemPtr(iAreaDrag);
			if(objeto == area->visualLimit[ENEditorAtlasRule_Left] || objeto == area->visualLimit[ENEditorAtlasRule_Right] || objeto == area->visualLimit[ENEditorAtlasRule_Top] || objeto == area->visualLimit[ENEditorAtlasRule_Bottom]){
				this->privRemoveEmptyAreas();
				this->privUpdateAreasRepresentations(EDITOR_ATLAS_UPDATE_MASK_SHAPES | EDITOR_ATLAS_UPDATE_MASK_BITMAPS_IF_NECESARY);
			}
		}
	}
}

//HOVER
void AUEditorAtlas::hoverIniciado(AUEscenaObjeto* objeto){
	const SI32 iAreaHover = privIndexAreaWithObject(objeto);
	if(iAreaHover != -1){
		STEditorAtlasArea* area = _atlasAreas->elemPtr(iAreaHover);
		SI32 j, limitsUpdated = 0;
		for(j = 0; j < ENEditorAtlasRule_Count; j++){
			if(objeto == area->visualLimit[j]){
				NBASSERT(area->visualLimitHoverCount[j] >= 0)
				area->visualLimitHoverCount[j]++;
				limitsUpdated++;
			}
		}
		if(limitsUpdated != 0){
			this->privUpdateAreasRepresentations(0);
		}
	}
}

void AUEditorAtlas::hoverMovido(const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	if(objeto == _layerDrag){
		if(_atlasSprite != NULL && _atlasMapaBits != NULL){
			const NBCajaAABB boxLocal = _atlasSprite->cajaAABBLocal();
			const NBPunto posLocal = _atlasSprite->coordenadaEscenaALocal(posActualEscena.x, posActualEscena.y);
			if(posLocal.x < boxLocal.xMin || posLocal.x >= boxLocal.xMax || posLocal.y < boxLocal.yMin || posLocal.y >= boxLocal.yMax){
				//Out of image
				_colorTxt->establecerVisible(false);
				_colorBorder->establecerVisible(false);
				_colorSample->establecerVisible(false);
			} else {
				//Inside image
				const MapaDeBitsDesciptor props = _atlasMapaBits->propiedades();
				SI32 xPix = (float)props.ancho * ((posLocal.x - boxLocal.xMin) / (boxLocal.xMax - boxLocal.xMin));
				SI32 yPix = (float)props.alto * (1.0f - ((posLocal.y - boxLocal.yMin) / (boxLocal.yMax - boxLocal.yMin))); //y-inverted
				NBASSERT(xPix >= 0 && yPix >= 0)
				if(xPix < 0) xPix = 0;
				if(yPix < 0) yPix = 0;
				if(xPix >= props.ancho) xPix = props.ancho - 1;
				if(yPix >= props.alto) yPix = props.alto - 1;
				const NBColor8 color = _atlasMapaBits->getPixel(xPix, yPix);
				//
				{
					_colorTxt->establecerVisible(true);
					_colorBorder->establecerVisible(true);
					_colorSample->establecerVisible(true);
					_colorPxPos.x	= xPix;
					_colorPxPos.y	= yPix;
					_colorValue		= color;
					_colorSample->establecerMultiplicadorColor8(color);
					{
						STNBString strColor;
						NBString_init(&strColor);
						NBString_concat(&strColor, "rgba(");
						NBString_concatSI32(&strColor, color.r);
						NBString_concat(&strColor, ", ");
						NBString_concatSI32(&strColor, color.g);
						NBString_concat(&strColor, ", ");
						NBString_concatSI32(&strColor, color.b);
						NBString_concat(&strColor, ", ");
						NBString_concatSI32(&strColor, color.a);
						NBString_concat(&strColor, ")");
						_colorTxt->establecerTexto(strColor.str);
						NBString_release(&strColor);
					}
				}
				//PRINTF_INFO("Atlas px(%d / %d, %d / %d) = rgba(%d, %d, %d, %d).\n", xPix, props.ancho, props.alto, yPix, color.r, color.g, color.b, color.a);
			}
		}
	}
}

void AUEditorAtlas::hoverFinalizado(AUEscenaObjeto* objeto){
	const SI32 iAreaHover = privIndexAreaWithObject(objeto);
	if(iAreaHover != -1){
		STEditorAtlasArea* area = _atlasAreas->elemPtr(iAreaHover);
		SI32 j, limitsUpdated = 0;
		for(j = 0; j < ENEditorAtlasRule_Count; j++){
			if(objeto == area->visualLimit[j]){
				NBASSERT(area->visualLimitHoverCount[j] > 0)
				area->visualLimitHoverCount[j]--;
				limitsUpdated++;
			}
		}
		if(limitsUpdated != 0){
			this->privUpdateAreasRepresentations(0);
		}
	}
}

//TECLAS (old)
bool AUEditorAtlas::teclaPresionada(SI32 codigoTecla){
	return false;
}

bool AUEditorAtlas::teclaLevantada(SI32 codigoTecla){
	return false;
}

bool AUEditorAtlas::teclaEspecialPresionada(SI32 codigoTecla){
	return false;
}

//TECLAS

void AUEditorAtlas::textEditorCloneInterfaceObjectsRetained(AUObjeto** dstVisualObj, ITextInputListener** dstVisualItf, AUObjeto** dstValueObj, ITextInputListener** dstValueItf){
	if(dstVisualObj != NULL){
		*dstVisualObj = _textEditor;
		_textEditor->retener();
	}
	if(dstVisualItf != NULL){
		*dstVisualItf = _textEditor;
	}
	if(dstValueObj != NULL){
		*dstValueObj = _textEditor;
		_textEditor->retener();
	}
	if(dstValueItf != NULL){
		*dstValueItf = _textEditor;
	}
}

STTextEditorSyncResult AUEditorAtlas::textEditorCompareClones(AUObjeto* visualClone, AUObjeto* valueClone){
	STTextEditorSyncResult r;
	NBMemory_setZero(r);
	r.synced = ENTextEditorSyncResult_None;
	if(valueClone != NULL){
		NBASSERT(valueClone->esClase(AUEscenaTextoEditableOffscreen::idTipoClase))
		if(valueClone->esClase(AUEscenaTextoEditableOffscreen::idTipoClase)){
			AUEscenaTextoEditableOffscreen* strEditor = (AUEscenaTextoEditableOffscreen*)valueClone;
			const char* str = strEditor->texto();
			if(str[0] != '\0'){
				r.cursorChanged = TRUE;
				r.valueChanged = TRUE;
				r.synced = ENTextEditorSyncResult_ListenerSynced;
			}
		}
	}
	return r;
}

STTextEditorSyncResult AUEditorAtlas::textEditorSyncClones(AUObjeto* visualClone, AUObjeto* valueClone){
	STTextEditorSyncResult r;
	NBMemory_setZero(r);
	r.synced = ENTextEditorSyncResult_None;
	if(valueClone != NULL){
		NBASSERT(valueClone->esClase(AUEscenaTextoEditableOffscreen::idTipoClase))
		if(valueClone->esClase(AUEscenaTextoEditableOffscreen::idTipoClase)){
			AUEscenaTextoEditableOffscreen* strEditor = (AUEscenaTextoEditableOffscreen*)valueClone;
			const char* str = strEditor->texto();
			if(str[0] != '\0'){
				SI32 i = 0;
				while(str[i] != '\0'){
					this->teclaIntroducirTexto(&str[i]);
					i++;
				}
				strEditor->vaciarTexto();
				r.cursorChanged = TRUE;
				r.valueChanged = TRUE;
				r.synced = ENTextEditorSyncResult_ListenerSynced;
			}
		}
	}
	return r;
}

void AUEditorAtlas::textEditorFocusGained(){
	//
}

void AUEditorAtlas::textEditorFocusLost(const ENNBKeybFocusLostType actionType){
	//
}

bool AUEditorAtlas::textEditorMantainBeforeTouchConsumer(AUEscenaObjeto* touchConsumer){
	return true;
}

//TECLAS
void AUEditorAtlas::teclaFocoObtenido(){
	//
}

void AUEditorAtlas::teclaFocoPerdido(){
	//
}

bool AUEditorAtlas::teclaMantenerFocoAnteConsumidorDeTouch(AUEscenaObjeto* consumidorTouch){
	return true;
}

bool AUEditorAtlas::teclaTieneTexto(){
	return false;
}

void AUEditorAtlas::teclaBackspace(){
	//
}

void AUEditorAtlas::teclaIntroducirTexto(const char* textoIntroducir){
	bool consumed = false;
	switch (textoIntroducir[0]) {
		case '+':
			//Scale
			if(!consumed){
				this->privWorkspaceSetScale(_workspaceScale * 1.1f);
			}
			break;
		case '-':
			//Scale
			if(!consumed){
				this->privWorkspaceSetScale(_workspaceScale * 0.9f);
			}
			break;
		case 'g':
		case 'G':
			//Save
			if(!consumed){
				_guiMenu->optionAnimate(ENEditorAtlasButton_Save, this->privAtlasSave());
			}
			break;
		case 't':
		case 'T':
			//Change texture color
			if(!consumed){
				this->privTextureSwitchShowColor();
				_guiMenu->optionAnimate(ENEditorAtlasButton_TextureMode, true);
			}
			break;
	}
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUEditorAtlas)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUEditorAtlas, "AUEditorAtlas")
AUOBJMETODOS_CLONAR_NULL(AUEditorAtlas)

