//
//  AUEditorMesh.cpp
//  app-refranero
//
//  Created by Marcos Ortega on 14/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#include "NoteuCorePrecomp.h"
#include "AUEditorMesh.h"
//

AUEditorMesh::AUEditorMesh(const SI32 iEscena) : AUAppEscena()
	, _iEscena(iEscena)
    , _listener(NULL)
    , _animObjetos(NULL)
	, _layerMain(NULL)
	, _layerEditor(NULL)
	, _layerPreview(NULL)
	, _layerDrag(NULL)
    , _layerGUI(NULL)
	, _enPrimerPlano(false)
	, _workspaceScale(1.0f)
    , _meshCoordMutliplier(8)
	, _meshFilePath(NULL)
    , _meshBitmap(NULL)
    , _meshBitmapExtraChuncks(NULL)
	, _meshTexture(NULL)
    , _meshMap(NULL)
	, _meshSprite(NULL)
	//
	, _guiMenu(NULL)
	//
	, _selMeshRangeOrientation(ENEditorMeshGrid_Count)
	, _selMeshRangeIndex(-1)
    , _selMeshRangeHoverMask(0)
	//
	, _previewSprite(NULL)
	, _previewSpriteDbg(NULL)
	, _previewGrowingW(true)
	, _previewGrowingH(true)
{
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUEditorMesh")
	//
	_animObjetos	= new(this) AUAnimadorObjetoEscena();
	_layerMain		= new(this) AUEscenaContenedor();
	_layerEditor	= new(this) AUEscenaContenedor();
	_layerPreview	= new(this) AUEscenaContenedor();
	_layerDrag		= new(this) AUEscenaContenedor();
	_layerGUI		= new(this) AUEscenaContenedor();
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
		_layerMain->agregarObjetoEscena(_layerEditor);
	}
	//Spite layer
	{
		_meshSpriteLayer = new(this) AUEscenaContenedor();
		_layerEditor->agregarObjetoEscena(_meshSpriteLayer);
	}
	//Mesh external rules
	{
		SI32 i;
		for(i = 0; i < ENEditorMeshRule_Count; i++){
			_meshRules[i] = new(this) AUArregloMutable();
		}
		_meshRulesLayer	= new(this) AUEscenaContenedor();
		_layerEditor->agregarObjetoEscena(_meshRulesLayer);
	}
	//Mesh internal grid
	{
		SI32 i;
		for(i = 0; i < ENEditorMeshGrid_Count; i++){
			_meshGrid[i] = new(this) AUArregloMutable();
		}
		_meshGridLayer	= new(this) AUEscenaContenedor();
		_layerEditor->agregarObjetoEscena(_meshGridLayer);
	}
	//Mesh ranges
	{
		SI32 i;
		for(i = 0; i < ENEditorMeshGrid_Count; i++){
			_meshRanges[i] = new(this) AUArregloNativoMutableP<STEditorMeshRange>();
		}
		_meshRangesLayer	= new(this) AUEscenaContenedor();
		_layerEditor->agregarObjetoEscena(_meshRangesLayer);
	}
	//Buttons
	{
		_guiMenu = new(this) AUIMenu();
		_guiMenu->setListener(this);
		_layerGUI->agregarObjetoEscena(_guiMenu);
		//
		SI32 i;
		for(i = 0; i  < ENEditorMeshButton_Count; i++){
			switch (i) {
				case ENEditorMeshButton_Close:	_guiMenu->optionAdd("CLOSE"); break;
				case ENEditorMeshButton_Save:	_guiMenu->optionAdd("SAVE"); break;
				default: _guiMenu->optionAdd("BUTTON"); NBASSERT(false) break;
			}
		}
	}
	//
	NBPUNTO_ESTABLECER(_workspaceCenter, 0.0f, 0.0f);
	//
	NBGestorEscena::establecerColorFondo(_iEscena, 0.1f, 0.1f, 0.1f, 1.0f);
	NBGestorEscena::normalizaCajaProyeccionEscena(_iEscena);
	NBGestorEscena::normalizaMatricesCapasEscena(_iEscena);
	_enPrimerPlano	= false;
	this->escenaColocarEnPrimerPlano();
	//
	this->privOrganizarContenido();
	NBGestorAnimadores::agregarAnimador(this, this);
	//
}

AUEditorMesh::~AUEditorMesh(){
	NBGestorAnimadores::quitarAnimador(this);
	//
	this->escenaQuitarDePrimerPlano();
	if(_animObjetos!=NULL) _animObjetos->liberar(NB_RETENEDOR_THIS); _animObjetos = NULL;
	if(_layerGUI != NULL) _layerGUI->liberar(NB_RETENEDOR_THIS); _layerGUI = NULL;
	if(_layerDrag != NULL) _layerDrag->liberar(NB_RETENEDOR_THIS); _layerDrag = NULL;
	if(_layerPreview != NULL) _layerPreview->liberar(NB_RETENEDOR_THIS); _layerPreview = NULL;
	if(_layerEditor != NULL) _layerEditor->liberar(NB_RETENEDOR_THIS); _layerEditor = NULL;
	if(_layerMain != NULL) _layerMain->liberar(NB_RETENEDOR_THIS); _layerMain = NULL;
	if(_textEditor != NULL) _textEditor->liberar(NB_RETENEDOR_THIS); _textEditor = NULL;
	//
	if(_previewSprite != NULL) _previewSprite->liberar(NB_RETENEDOR_THIS); _previewSprite = NULL;
	if(_previewSpriteDbg != NULL) _previewSpriteDbg->liberar(NB_RETENEDOR_THIS); _previewSpriteDbg = NULL;
	//
	this->privMeshClose();
	if(_meshSpriteLayer != NULL) _meshSpriteLayer->liberar(); _meshSpriteLayer = NULL;
	if(_guiMenu != NULL) _guiMenu->liberar(); _guiMenu = NULL;
	//Mesh external rules (asuming 'privMeshClose' relased all objects)
	{
		SI32 i;
		for(i = 0; i < ENEditorMeshRule_Count; i++){
			_meshRules[i]->liberar();
			_meshRules[i] = NULL;
		}
		if(_meshRulesLayer != NULL) _meshRulesLayer->liberar(); _meshRulesLayer = NULL;
	}
	//Mesh internal grid (asuming 'privMeshClose' relased all objects)
	{
		SI32 i;
		for(i = 0; i < ENEditorMeshGrid_Count; i++){
			_meshGrid[i]->liberar();
			_meshGrid[i] = NULL;
		}
		if(_meshGridLayer != NULL) _meshGridLayer->liberar(); _meshGridLayer = NULL;
	}
	//Mesh ranges (asuming 'privMeshClose' relased all objects)
	{
		SI32 i;
		for(i = 0; i < ENEditorMeshGrid_Count; i++){
			_meshRanges[i]->liberar();
			_meshRanges[i] = NULL;
		}
		if(_meshRangesLayer != NULL) _meshRangesLayer->liberar(); _meshRangesLayer = NULL;
	}
}

//

bool AUEditorMesh::escenaEnPrimerPlano(){
	return _enPrimerPlano;
}

void AUEditorMesh::escenaColocarEnPrimerPlano(){
	if(!_enPrimerPlano){
		NBColor8 coloBlanco; NBCOLOR_ESTABLECER(coloBlanco, 255, 255, 255, 255);
		NBGestorEscena::agregarObjetoCapa(_iEscena, ENGestorEscenaGrupo_Escena, _layerMain, coloBlanco);
		NBGestorEscena::agregarObjetoCapa(_iEscena, ENGestorEscenaGrupo_GUI, _layerGUI, coloBlanco);
		NBGestorEscena::establecerColorFondo(_iEscena, 0.1f, 0.1f, 0.1f, 1.0f);
		NBGestorEscena::agregarEscuchadorCambioPuertoVision(_iEscena, this);
		NBGestorAnimadores::agregarAnimador(this, this);
		//
		NBGestorTeclas::escuchadorEstablecer(this, this);
		//
		this->privOrganizarContenido();
		if(_animObjetos!=NULL) _animObjetos->reanudarAnimacion();
		_enPrimerPlano = true;
	}
}

void AUEditorMesh::escenaQuitarDePrimerPlano(){
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

bool AUEditorMesh::escenaPermitidoGirarPantalla(){
	return true;
}

bool AUEditorMesh::escenaEstaAnimandoSalida(){
	if(_animObjetos!=NULL) return (_animObjetos->conteoAnimacionesEjecutando()!=0);
	return false;
}

void AUEditorMesh::escenaAnimarEntrada(){
	//
}

void AUEditorMesh::escenaAnimarSalida(){
	//
}

void AUEditorMesh::escenaGetOrientations(UI32* dstMask, ENAppOrientationBit* dstPrefered, ENAppOrientationBit* dstToApplyOnce, BOOL* dstAllowAutoRotate){
	//
}

void AUEditorMesh::appFocoExclusivoObtenido(){
	//
}

void AUEditorMesh::appFocoExclusivoPerdido(){
	//
}

void AUEditorMesh::appProcesarNotificacionLocal(const char* grp, const SI32 localId, const char* objTip, const SI32 objId){
	//
}

//Listener

IListenerEditorMesh* AUEditorMesh::listener(){
	return _listener;
}

void AUEditorMesh::setListener(IListenerEditorMesh* listener){
	_listener = listener;
}

//

AUMapaBits* AUEditorMesh::meshBitmap(){
	return _meshBitmap;
}

AUMallaMapa* AUEditorMesh::meshMap(){
	return _meshMap;
}

//

bool AUEditorMesh::openFile(const char* path){
	bool r = this->privMeshOpen(path);
	return r;
}

bool AUEditorMesh::openBitmap(AUMapaBits* bitmap, AUMallaMapa* meshMap){
	bool r = this->privMeshLoadFromBitmap(bitmap, meshMap);
	return r;
}

bool AUEditorMesh::privMeshOpen(const char* imageFilePath){
	bool r = false;
	//
	if(imageFilePath != NULL){
		AUMapaBitsMutable* mapaBits = new(this) AUMapaBitsMutable();
		AUPngChuncksMutable* extraChuncks = new(this) AUPngChuncksMutable();
		if(!mapaBits->cargarDesdeArchivoPNG(imageFilePath, true, NULL, extraChuncks)){
			PRINTF_ERROR("Couldnt load image file: '%s'.\n", imageFilePath);
		} else {
			AUTextura* meshTexture = NBGestorTexturas::texturaDesdeMapaBits(mapaBits, "", ENTexturaModoPintado_Imagen_Precisa, ENTexturaMipMap_Inhabilitado, ENTexturaOrdenV_HaciaArriba, _meshCoordMutliplier, 1.0f);
			if(meshTexture == NULL){
				PRINTF_ERROR("File loaded but couldnt create texture: '%s'.\n", imageFilePath);
			} else {
				//
				this->privMeshClose();
				//
				_meshTexture	= meshTexture; meshTexture->retener();
				_meshBitmap		= mapaBits; mapaBits->retener();
				_meshBitmapExtraChuncks = extraChuncks; extraChuncks->retener();
				_meshMap		= NULL;
				_meshSprite		= new(this) AUEscenaSprite(_meshTexture, ENSpriteAlineaH_Der, ENSpriteAlineaV_Arr);
				_meshFilePath	= new(this) AUCadena8(imageFilePath);
				_meshSpriteLayer->agregarObjetoEscena(_meshSprite);
				//Create new atlas and load from chuncks-data (if available)
				{
					AUMallaMapaMutable* meshMap = new(this) AUMallaMapaMutable(1);
					const AUCadenaLarga8* chunckAT = extraChuncks->getChunckById("nbMH");
					if(chunckAT != NULL){
						AUArchivoEnBuffer* flujo = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer(&chunckAT->str()[4], chunckAT->tamano() - 4);
						flujo->lock();
						if(!AUMallaMapa::cargarXmlEn(meshMap, flujo)){
							PRINTF_ERROR("Could not load mesh from png data chunck: (%d bytes).\n", chunckAT->tamano());
						} else {
							PRINTF_INFO("Mesh loaded from png data chunck: (%d bytes).\n", chunckAT->tamano());
							_meshMap = meshMap; meshMap->retener();
						}
						flujo->unlock();
						flujo->liberar();
					}
				}
				//
				const NBTamano sizeHD = _meshTexture->tamanoHD();
				NBTamanoI sizePix; NBTAMANO_ESTABLECER(sizePix, sizeHD.ancho, sizeHD.alto)
				//Add initial ranges
				if(_meshMap == NULL){
					_meshMap		= new(this) AUMallaMapaMutable(8);
					_meshMap->agregarCorte(ENMallaCorteSentido_Vertical, sizePix.ancho, ENMallaCorteTipo_Rigido);
					_meshMap->agregarCorte(ENMallaCorteSentido_Horizontal, sizePix.alto, ENMallaCorteTipo_Rigido);
				}
				//Ensure at l;east one cur per direction
				if(_meshMap->cortesEnSentido(ENMallaCorteSentido_Vertical)->conteo == 0){
					_meshMap->agregarCorte(ENMallaCorteSentido_Vertical, sizePix.ancho, ENMallaCorteTipo_Rigido);
				}
				if(_meshMap->cortesEnSentido(ENMallaCorteSentido_Horizontal)->conteo == 0){
					_meshMap->agregarCorte(ENMallaCorteSentido_Horizontal, sizePix.alto, ENMallaCorteTipo_Rigido);
				}
				//
				NBASSERT(_meshRules[ENEditorMeshRule_Left]->conteo == 0 && _meshRules[ENEditorMeshRule_Right]->conteo == 0 && _meshRules[ENEditorMeshRule_Top]->conteo == 0 && _meshRules[ENEditorMeshRule_Bottom]->conteo == 0)
				this->privInitGrid(sizePix.ancho, sizePix.alto, 0.0f);
				this->privWorkspaceSetScale(_workspaceScale);
				this->privUpdateRangesRepresentations(true);
				r = true;
			}
		}
		extraChuncks->liberar();
		mapaBits->liberar();
	}
	return r;
}

bool AUEditorMesh::privMeshLoadFromBitmap(AUMapaBits* bitmap, AUMallaMapa* meshMap){
	bool r = false;
	if(bitmap != NULL){
		AUMapaBitsMutable* mapaBits = new(this) AUMapaBitsMutable(bitmap);
		AUTextura* meshTexture = NBGestorTexturas::texturaDesdeMapaBits(mapaBits, "", ENTexturaModoPintado_Imagen_Precisa, ENTexturaMipMap_Inhabilitado, ENTexturaOrdenV_HaciaArriba, _meshCoordMutliplier, 1.0f);
		if(meshTexture == NULL){
			PRINTF_ERROR("Couldnt create texture.\n");
		} else {
			//
			this->privMeshClose();
			//
			_meshTexture	= meshTexture; meshTexture->retener();
			_meshBitmap		= mapaBits; mapaBits->retener();
			_meshBitmapExtraChuncks = new(this) AUPngChuncksMutable();
			if(meshMap == NULL){
				_meshMap		= new(this) AUMallaMapaMutable(8);
			} else {
				_meshMap		= new(this) AUMallaMapaMutable(meshMap);
			}
			_meshSprite		= new(this) AUEscenaSprite(_meshTexture, ENSpriteAlineaH_Der, ENSpriteAlineaV_Arr);
			_meshFilePath	= NULL; //new(this) AUCadena8(imageFilePath);
			_meshSpriteLayer->agregarObjetoEscena(_meshSprite);
			//
			const NBTamano sizeHD = _meshTexture->tamanoHD();
			NBTamanoI sizePix; NBTAMANO_ESTABLECER(sizePix, sizeHD.ancho, sizeHD.alto)
			//Add initial ranges
			if(_meshMap == NULL){
				_meshMap		= new(this) AUMallaMapaMutable(8);
				_meshMap->agregarCorte(ENMallaCorteSentido_Vertical, sizePix.ancho, ENMallaCorteTipo_Rigido);
				_meshMap->agregarCorte(ENMallaCorteSentido_Horizontal, sizePix.alto, ENMallaCorteTipo_Rigido);
			}
			//Ensure at least one cut per direction
			if(_meshMap->cortesEnSentido(ENMallaCorteSentido_Vertical)->conteo == 0){
				_meshMap->agregarCorte(ENMallaCorteSentido_Vertical, sizePix.ancho, ENMallaCorteTipo_Rigido);
			}
			if(_meshMap->cortesEnSentido(ENMallaCorteSentido_Horizontal)->conteo == 0){
				_meshMap->agregarCorte(ENMallaCorteSentido_Horizontal, sizePix.alto, ENMallaCorteTipo_Rigido);
			}
			//
			NBASSERT(_meshRules[ENEditorMeshRule_Left]->conteo == 0 && _meshRules[ENEditorMeshRule_Right]->conteo == 0 && _meshRules[ENEditorMeshRule_Top]->conteo == 0 && _meshRules[ENEditorMeshRule_Bottom]->conteo == 0)
			this->privInitGrid(sizePix.ancho, sizePix.alto, 0.0f);
			this->privWorkspaceSetScale(_workspaceScale);
			this->privUpdateRangesRepresentations(true);
			r = true;
		}
		mapaBits->liberar();
	}
	return r;
}

void AUEditorMesh::privInitGrid(const SI32 ancho, const SI32 alto, const float marginFromMesh){
	//Add horizontal external rules / internal grid
	SI32 p;
	for(p = 0; p < ancho; p++){
		SI32 lineLen; NBColor8 ruleColor, gridColor; AUEscenaFigura* fig;
		if((p % _meshCoordMutliplier) == 0){
			lineLen = 8;
			NBCOLOR_ESTABLECER(ruleColor, 0, 255, 0, 255)
			NBCOLOR_ESTABLECER(gridColor, 150, 150, 150, 255)
		} else if((p % (_meshCoordMutliplier / 2)) == 0){
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
		fig->establecerTraslacion(p, alto + marginFromMesh);
		fig->establecerMultiplicadorColor8(ruleColor);
		_meshRules[ENEditorMeshRule_Top]->agregarElemento(fig);
		_meshRulesLayer->agregarObjetoEscena(fig);
		fig->liberar();
		//Down
		fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
		fig->agregarCoordenadaLocal(0.0f, 0.0f);
		fig->agregarCoordenadaLocal(0.0f, -lineLen);
		fig->establecerTraslacion(p, -marginFromMesh);
		fig->establecerMultiplicadorColor8(ruleColor);
		_meshRules[ENEditorMeshRule_Bottom]->agregarElemento(fig);
		_meshRulesLayer->agregarObjetoEscena(fig);
		fig->liberar();
		//Vertical (grid)
		fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
		fig->agregarCoordenadaLocal(0.0f, 0.0f);
		fig->agregarCoordenadaLocal(0.0f, alto);
		fig->establecerTraslacion(p, 0.0f);
		fig->establecerMultiplicadorColor8(gridColor);
		_meshGrid[ENEditorMeshGrid_CutV]->agregarElemento(fig);
		_meshGridLayer->agregarObjetoEscena(fig);
		fig->liberar();
	}
	//Add vertical external rules / internal grid
	for(p = 0; p < alto; p++){
		SI32 lineLen; NBColor8 ruleColor, gridColor; AUEscenaFigura* fig;
		if((p % _meshCoordMutliplier) == 0){
			lineLen = 8;
			NBCOLOR_ESTABLECER(ruleColor, 0, 255, 0, 255)
			NBCOLOR_ESTABLECER(gridColor, 150, 150, 150, 255)
		} else if((p % (_meshCoordMutliplier / 2)) == 0){
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
		fig->establecerTraslacion(ancho + marginFromMesh, p);
		fig->establecerMultiplicadorColor8(ruleColor);
		_meshRules[ENEditorMeshRule_Right]->agregarElemento(fig);
		_meshRulesLayer->agregarObjetoEscena(fig);
		fig->liberar();
		//Left
		fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
		fig->agregarCoordenadaLocal(0.0f, 0.0f);
		fig->agregarCoordenadaLocal(-lineLen, 0.0f);
		fig->establecerTraslacion(-marginFromMesh, p);
		fig->establecerMultiplicadorColor8(ruleColor);
		_meshRules[ENEditorMeshRule_Left]->agregarElemento(fig);
		_meshRulesLayer->agregarObjetoEscena(fig);
		fig->liberar();
		//Horizontal (grid)
		fig = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
		fig->agregarCoordenadaLocal(0.0f, 0.0f);
		fig->agregarCoordenadaLocal(ancho, 0.0f);
		fig->establecerTraslacion(0.0f, p);
		fig->establecerMultiplicadorColor8(gridColor);
		_meshGrid[ENEditorMeshGrid_CutH]->agregarElemento(fig);
		_meshGridLayer->agregarObjetoEscena(fig);
		fig->liberar();
	}
}

void AUEditorMesh::privMeshClose(){
	if(_meshFilePath != NULL) _meshFilePath->liberar(); _meshFilePath = NULL;
	if(_meshBitmap != NULL) _meshBitmap->liberar(); _meshBitmap = NULL;
	if(_meshBitmapExtraChuncks != NULL) _meshBitmapExtraChuncks->liberar(); _meshBitmapExtraChuncks = NULL;
	if(_meshTexture != NULL) _meshTexture->liberar(); _meshTexture = NULL;
	if(_meshMap != NULL) _meshMap->liberar(); _meshMap = NULL;
	if(_meshSprite != NULL){
		AUEscenaContenedor* parent = (AUEscenaContenedor*)_meshSprite->contenedor();
		if(parent != NULL){
			parent->quitarObjetoEscena(_meshSprite);
		}
		_meshSprite->liberar();
		_meshSprite = NULL;
	}
	//
	this->privSelectionSet(ENEditorMeshGrid_Count, -1);
	//Mesh extenral rules
	{
		SI32 i, j;
		for(i = 0; i < ENEditorMeshRule_Count; i++){
			const SI32 linesCount = _meshRules[i]->conteo;
			for(j = 0; j < linesCount; j++){
				AUEscenaFigura* fig = (AUEscenaFigura*)_meshRules[i]->elem(j);
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != NULL){
					parent->quitarObjetoEscena(fig);
				}
			}
			_meshRules[i]->vaciar();
		}
	}
	//Mesh internal grid
	{
		SI32 i, j;
		for(i = 0; i < ENEditorMeshGrid_Count; i++){
			const SI32 linesCount = _meshGrid[i]->conteo;
			for(j = 0; j < linesCount; j++){
				AUEscenaFigura* fig = (AUEscenaFigura*)_meshGrid[i]->elem(j);
				AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
				if(parent != NULL){
					parent->quitarObjetoEscena(fig);
				}
			}
			_meshGrid[i]->vaciar();
		}
	}
	//Mesh Ranges
	{
		SI32 i, j;
		for(i = 0; i < ENEditorMeshGrid_Count; i++){
			const SI32 linesCount = _meshRanges[i]->conteo;
			for(j = 0; j < linesCount; j++){
				STEditorMeshRange* datRange = _meshRanges[i]->elemPtr(j);
				if(datRange->visualRange != NULL){
					AUEscenaFigura* fig = datRange->visualRange;
					AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
					if(parent != NULL){
						parent->quitarObjetoEscena(fig);
					}
					fig->liberar();
					datRange->visualRange = NULL;
				}
				if(datRange->visualMin != NULL){
					AUEscenaFigura* fig = datRange->visualMin;
					AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
					if(parent != NULL){
						parent->quitarObjetoEscena(fig);
					}
					fig->liberar();
					datRange->visualMin = NULL;
				}
				if(datRange->visualMax != NULL){
					AUEscenaFigura* fig = datRange->visualMax;
					AUEscenaContenedor* parent = (AUEscenaContenedor*)fig->contenedor();
					if(parent != NULL){
						parent->quitarObjetoEscena(fig);
					}
					fig->liberar();
					datRange->visualMax = NULL;
				}
			}
			_meshRanges[i]->vaciar();
		}
	}
}

bool AUEditorMesh::privMeshSaveAction(){
	bool r = false;
	if(_listener != NULL){
		r = _listener->editorMeshSaveCommand(this);
	} else {
		r = this->privMeshSaveFile();
	}
	return r;
}

bool AUEditorMesh::privMeshSaveFile(){
	bool r = false;
	if(_meshFilePath != NULL && _meshMap != NULL && _meshBitmap != NULL){
		if(_meshFilePath->tamano() > 0){
			AUArchivoEnBuffer* flujo = new(ENMemoriaTipo_Temporal) AUArchivoEnBuffer();
			flujo->lock();
			if(!AUMallaMapa::guardarEnXml(_meshMap, flujo)){
				PRINTF_ERROR("Could not generate xml of mesh.\n");
			} else {
				const AUCadena8* buffStream = flujo->buffer();
				//Save png-chunck
				_meshBitmapExtraChuncks->setChunck("nbMH", (const BYTE*)buffStream->str(), buffStream->tamano());
				//Save atlas backup file
				this->saveMeshBackupFile(_meshMap, _meshFilePath->str());
				//Save png
				if(!_meshBitmap->guardarComoPNG(_meshFilePath->str(), false, NULL, NULL, NULL, _meshBitmapExtraChuncks)){
					PRINTF_ERROR("Could not save to '%s'.\n", _meshFilePath->str());
				} else {
					PRINTF_INFO("Atlas saved into '%s': (%d bytes in xml).\n", _meshFilePath->str(), buffStream->tamano());
					r = true;
				}
			}
			flujo->unlock();
			flujo->liberar();
		}
	}
	return r;
}

bool AUEditorMesh::saveMeshBackupFile(AUMallaMapa* meshMap, const char* origFilePath){
	bool r = false;
	if(origFilePath != NULL){
		if(origFilePath[0] != '\0'){
			AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8(origFilePath);
			strTmp->agregar(".mesh.bak");
			AUArchivo* flujo = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, strTmp->str(), ENArchivoModo_SoloEscritura);
			if(flujo == NULL){
				PRINTF_ERROR("Could not open file for writting: '%s'.\n", strTmp->str());
			} else {
				flujo->lock();
				if(!AUMallaMapa::guardarEnXml(meshMap, flujo)){
					PRINTF_ERROR("Could not generate xml of atlas.\n");
				} else {
					PRINTF_INFO("Atlas-backup saved into '%s'.\n", strTmp->str());
					r = true;
				}
				flujo->unlock();
				flujo->cerrar();
			}
			strTmp->liberar();
		}
	}
	return r;
}

//

SI32 AUEditorMesh::privIndexOfRangeWithVisualObject(AUEscenaObjeto* obj, const ENEditorMeshGrid rangeOrientation){
	SI32 r = -1;
	if(rangeOrientation < ENEditorMeshGrid_Count){
		AUArregloNativoP<STEditorMeshRange>* ranges = _meshRanges[rangeOrientation];
		SI32 i; const SI32 count = ranges->conteo;
		for(i = 0; i < count; i++){
			STEditorMeshRange* range = ranges->elemPtr(i);
			if(range->visualMin == obj || range->visualMax == obj || range->visualRange == obj){
				r = i;
				break;
			}
		}
	}
	return r;
}

void AUEditorMesh::privSelectionSet(const ENEditorMeshGrid orientation, const SI32 index){
	//Disable current touch listeners
	if(_selMeshRangeOrientation < ENEditorMeshGrid_Count && _selMeshRangeIndex >= 0){
		AUArregloNativoMutableP<STEditorMeshRange>* meshRanges = _meshRanges[_selMeshRangeOrientation];
		NBASSERT(_selMeshRangeIndex < meshRanges->conteo)
		if(_selMeshRangeIndex < meshRanges->conteo){
			STEditorMeshRange* datRange	= meshRanges->elemPtr(_selMeshRangeIndex);
			NBASSERT(datRange->visualMin->escuchadorTouches() == this)
			NBASSERT(datRange->visualMax->escuchadorTouches() == this)
			datRange->visualMin->establecerEscuchadorTouches(NULL, NULL);
			datRange->visualMax->establecerEscuchadorTouches(NULL, NULL);
		}
	}
	//Enable new touch listeners
	if(orientation < ENEditorMeshGrid_Count && index >= 0){
		AUArregloNativoMutableP<STEditorMeshRange>* meshRanges = _meshRanges[orientation];
		NBASSERT(index < meshRanges->conteo)
		if(index < meshRanges->conteo){
			STEditorMeshRange* datRange	= meshRanges->elemPtr(index);
			NBASSERT(datRange->visualMin->escuchadorTouches() == NULL)
			NBASSERT(datRange->visualMax->escuchadorTouches() == NULL)
			datRange->visualMin->establecerEscuchadorTouches(this, this);
			datRange->visualMax->establecerEscuchadorTouches(this, this);
		}
	}
	//Set values
	_selMeshRangeOrientation	= orientation;
	_selMeshRangeIndex			= index;
	_selMeshRangeHoverMask		= 0;
}

//


bool AUEditorMesh::menuOptionExecuteAction(AUIMenu* obj, const SI32 optionId){
	bool r = false;
	if(_guiMenu == obj && _meshBitmap != NULL){
		switch (optionId) {
			case ENEditorMeshButton_Close:
				if(_listener != NULL){
					r = _listener->editorMeshCloseCommand(this);
				}
				break;
			case ENEditorMeshButton_Save:
				r = this->privMeshSaveAction();
				break;
			default:
				NBASSERT(false)
				break;
		}
	}
	return r;
}

//

void AUEditorMesh::puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after){
	_animObjetos->purgarAnimaciones();
	this->privOrganizarContenido();
}

void AUEditorMesh::privOrganizarContenido(){
	const NBRectangulo visionPort = NBGestorEscena::puertoDeVision(_iEscena);
	//
	NBCajaAABB workspaceBox;
	workspaceBox.xMin = _workspaceCenter.x - (visionPort.ancho * 0.5f / _workspaceScale);
	workspaceBox.xMax = _workspaceCenter.x + (visionPort.ancho * 0.5f / _workspaceScale);
	workspaceBox.yMin = _workspaceCenter.y - (visionPort.alto * 0.5f / _workspaceScale);
	workspaceBox.yMax = _workspaceCenter.y + (visionPort.alto * 0.5f / _workspaceScale);
	
	NBGestorEscena::establecerCajaProyeccion(_iEscena, workspaceBox.xMin, workspaceBox.xMax, workspaceBox.yMin, workspaceBox.yMax);
	//
	NBASSERT(_layerDrag->hijos()->conteo >= 2)
	if(_layerDrag->hijos()->conteo >= 2){
		AUEscenaObjeto* refObjTopRght	= (AUEscenaObjeto*)_layerDrag->hijos()->elem(0);
		AUEscenaObjeto* refObjBtmLeft	= (AUEscenaObjeto*)_layerDrag->hijos()->elem(1);
		refObjTopRght->establecerTraslacion(workspaceBox.xMin, workspaceBox.yMin);
		refObjBtmLeft->establecerTraslacion(workspaceBox.xMax, workspaceBox.yMax);
	}
	//GUI
	{
		const NBCajaAABB cajaGUI	= NBGestorEscena::cajaProyeccionGrupo(_iEscena, ENGestorEscenaGrupo_GUI);
		//Menu
		{
			_guiMenu->establecerTraslacion(cajaGUI.xMin + 8.0f, cajaGUI.yMax - 4.0f);
		}
	}
}

void AUEditorMesh::privWorkspaceSetScale(const float workspaceScale){
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
	for(i = 0; i < ENEditorMeshRule_Count; i++){
		const SI32 linesCount = _meshRules[i]->conteo;
		for(j = 0; j < linesCount; j++){
			AUEscenaFigura* fig = (AUEscenaFigura*)_meshRules[i]->elem(j);
			fig->establecerVisible((j % visibilityBase) == 0);
		}
	}
	_meshGridLayer->establecerVisible((_workspaceScale >= 4.0f));
	//
	this->privOrganizarContenido();
}

void AUEditorMesh::privUpdateRangesRepresentations(const bool pUpdateShapes){
	if(_meshTexture != NULL){
		bool updateShapes = pUpdateShapes;
		SI32 i, j; const NBTamano sizeHD = _meshTexture->tamanoHD();
		//Add or remove areas (if necesary)
		for(i = 0; i < ENEditorMeshGrid_Count; i++){
			const AUArregloNativoP<STMallaCorte>* cutsMap = _meshMap->cortesEnSentido((ENMallaCorteSentido)i);
			AUArregloNativoMutableP<STEditorMeshRange>* meshRanges = _meshRanges[i];
			//Add new areas (if necesary)
			if(cutsMap->conteo > meshRanges->conteo){
				//PRINTF_INFO("Adding new ranges-%d: map(%d) vs ranges(%d).\n", i, cutsMap->conteo, meshRanges->conteo);
				SI32 j; const SI32 conteo = cutsMap->conteo;
				for(j = meshRanges->conteo; j < conteo; j++){
					STEditorMeshRange datRange;
					datRange.visualRange = new(this) AUEscenaFigura(ENEscenaFiguraTipo_PoligonoCerrado);
					datRange.visualRange->agregarCoordenadaLocal(0.0f, 0.0f);
					datRange.visualRange->agregarCoordenadaLocal(0.0f, 0.0f);
					datRange.visualRange->agregarCoordenadaLocal(0.0f, 0.0f);
					datRange.visualRange->agregarCoordenadaLocal(0.0f, 0.0f);
					datRange.visualRange->establecerTraslacion(0.0f, 0.0f);
					//datRange.visualRange->establecerEscuchadorTouches(this, this);
					//
					datRange.visualMin = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
					datRange.visualMin->agregarCoordenadaLocal(0.0f, 0.0f);
					datRange.visualMin->agregarCoordenadaLocal(0.0f, 0.0f);
					datRange.visualMin->establecerTraslacion(0.0f, 0.0f);
					//
					datRange.visualMax = new(this) AUEscenaFigura(ENEscenaFiguraTipo_Linea);
					datRange.visualMax->agregarCoordenadaLocal(0.0f, 0.0f);
					datRange.visualMax->agregarCoordenadaLocal(0.0f, 0.0f);
					datRange.visualMax->establecerTraslacion(0.0f, 0.0f);
					//Set selection listeners
					NBASSERT(!(_selMeshRangeOrientation == i && _selMeshRangeIndex == j)) //New areas should be created with 'selected' status?
					if(_selMeshRangeOrientation == i && _selMeshRangeIndex == j){
						datRange.visualMin->establecerEscuchadorTouches(this, this);
						datRange.visualMax->establecerEscuchadorTouches(this, this);
					}
					//
					_meshRangesLayer->agregarObjetoEscena(datRange.visualRange);
					_meshRangesLayer->agregarObjetoEscena(datRange.visualMin);
					_meshRangesLayer->agregarObjetoEscena(datRange.visualMax);
					meshRanges->agregarElemento(datRange);
					//PRINTF_INFO("Range-%d added (%d total).\n", i, meshRanges->conteo);
				}
				updateShapes = true;
			}
			//Remove extra areas (if necesary)
			if(meshRanges->conteo > cutsMap->conteo){
				//PRINTF_INFO("Removing extra ranges-%d: map(%d) vs ranges(%d).\n", i, cutsMap->conteo, meshRanges->conteo);
				SI32 i; const SI32 conteo = cutsMap->conteo;
				for(i = meshRanges->conteo - 1; i >= conteo; i--){
					STEditorMeshRange* range = meshRanges->elemPtr(i);
					AUEscenaContenedor* container;
					container = (AUEscenaContenedor*)range->visualMin->contenedor(); if(container != NULL){ container->quitarObjetoEscena(range->visualMin); }
					container = (AUEscenaContenedor*)range->visualMax->contenedor(); if(container != NULL){ container->quitarObjetoEscena(range->visualMax); }
					container = (AUEscenaContenedor*)range->visualRange->contenedor(); if(container != NULL){ container->quitarObjetoEscena(range->visualRange); }
					range->visualMin->liberar(); range->visualMin = NULL;
					range->visualMax->liberar(); range->visualMax = NULL;
					range->visualRange->liberar(); range->visualRange = NULL;
					meshRanges->quitarElementoEnIndice(i);
					//PRINTF_INFO("Range-%d removed (%d total).\n", i, meshRanges->conteo);
				}
				updateShapes = true;
			}
		}
		//Update areas
		for(i = 0; i < ENEditorMeshGrid_Count; i++){
			const AUArregloNativoP<STMallaCorte>* cutsMap = _meshMap->cortesEnSentido((ENMallaCorteSentido)i);
			AUArregloNativoMutableP<STEditorMeshRange>* meshRanges = _meshRanges[i];
			NBASSERT(cutsMap->conteo == meshRanges->conteo)
			//
			float posHd = 0.0f;
			const SI32 linesCount = meshRanges->conteo;
			for(j = 0; j < linesCount; j++){
				STMallaCorte* datCut	= cutsMap->elemPtr(j);
				STEditorMeshRange* datRange	= meshRanges->elemPtr(j);
				const bool isSelection	= (_selMeshRangeOrientation == i && _selMeshRangeIndex == j);
				float posHdEnd			= posHd + datCut->tamano;
				if(i == ENEditorMeshGrid_CutH){
					if(posHdEnd > sizeHD.alto || (j + 1) == linesCount){
						posHdEnd = sizeHD.alto;
					}
				} else if(i == ENEditorMeshGrid_CutV){
					if(posHdEnd > sizeHD.ancho || (j + 1) == linesCount){
						posHdEnd = sizeHD.ancho;
					}
				}
				//fix-size
				float sizeHd = (posHdEnd - posHd);
				if(sizeHd != datCut->tamano){
					datCut->tamano		= sizeHd;
				}
				//
				if(datRange->visualRange != NULL){
					AUEscenaFigura* fig = datRange->visualRange;
					//Update color
					const UI8 alpha = (isSelection ? 155 : 50);
					if(datCut->tipo == ENMallaCorteTipo_Estirar){
						fig->establecerMultiplicadorColor8(50, 150, 50, alpha);
					} else if(datCut->tipo == ENMallaCorteTipo_Repetir){
						fig->establecerMultiplicadorColor8(50, 50, 150, alpha);
					} else {
						fig->establecerMultiplicadorColor8(150, 150, 150, alpha);
					}
					//Update shape
					if(updateShapes){
						if(i == ENEditorMeshGrid_CutH){
							datRange->visualRange->moverVerticeHacia(0, 0.0f, 0.0f);
							datRange->visualRange->moverVerticeHacia(1, sizeHD.ancho, 0.0f);
							datRange->visualRange->moverVerticeHacia(2, sizeHD.ancho, sizeHd);
							datRange->visualRange->moverVerticeHacia(3, 0.0f, sizeHd);
							datRange->visualRange->establecerTraslacion(0.0f, posHd);
						} else if(i == ENEditorMeshGrid_CutV){
							datRange->visualRange->moverVerticeHacia(0, 0.0f, 0.0f);
							datRange->visualRange->moverVerticeHacia(1, sizeHd, 0.0f);
							datRange->visualRange->moverVerticeHacia(2, sizeHd, sizeHD.alto);
							datRange->visualRange->moverVerticeHacia(3, 0.0f, sizeHD.alto);
							datRange->visualRange->establecerTraslacion(posHd, 0.0f);
						}
					}
				}
				if(datRange->visualMin != NULL){
					AUEscenaFigura* fig = datRange->visualMin;
					//Update color
					if((_selMeshRangeHoverMask & RANGE_MASK_ITEM_MIN) != 0){
						fig->establecerMultiplicadorColor8(0, 255, 0, isSelection ? 255 : 50);
					} else {
						fig->establecerMultiplicadorColor8(255, 255, 255, isSelection ? 255 : 50);
					}
					fig->establecerVisible(i != _selMeshRangeOrientation || _selMeshRangeIndex < 0 || j != (_selMeshRangeIndex + 1)); //Hide to allow hover over selected range limits.
					//Update shape
					if(updateShapes){
						if(i == ENEditorMeshGrid_CutH){
							datRange->visualMin->moverVerticeHacia(0, -2048.0f, 0.0f);
							datRange->visualMin->moverVerticeHacia(1, 2048.0f + sizeHD.ancho, 0.0f);
							datRange->visualMin->establecerTraslacion(0.0f, posHd);
						} else if(i == ENEditorMeshGrid_CutV){
							datRange->visualMin->moverVerticeHacia(0, 0.0f, -2048.0f);
							datRange->visualMin->moverVerticeHacia(1, 0.0f, 2048.0f + sizeHD.alto);
							datRange->visualMin->establecerTraslacion(posHd, 0.0f);
						}
					}
				}
				if(datRange->visualMax != NULL){
					AUEscenaFigura* fig = datRange->visualMax;
					//Update color
					if((_selMeshRangeHoverMask & RANGE_MASK_ITEM_MAX) != 0){
						fig->establecerMultiplicadorColor8(0, 255, 0, isSelection ? 255 : 50);
					} else {
						fig->establecerMultiplicadorColor8(255, 255, 255, isSelection ? 255 : 50);
					}
					fig->establecerVisible(i != _selMeshRangeOrientation || _selMeshRangeIndex < 0 || j != (_selMeshRangeIndex - 1)); //Hide to allow hover over selected range limits.
					//Update shape
					if(updateShapes){
						if(i == ENEditorMeshGrid_CutH){
							datRange->visualMax->moverVerticeHacia(0, -2048.0f, 0.0f);
							datRange->visualMax->moverVerticeHacia(1, 2048.0f + sizeHD.ancho, 0.0f);
							datRange->visualMax->establecerTraslacion(0.0f, posHdEnd);
						} else if(i == ENEditorMeshGrid_CutV){
							datRange->visualMax->moverVerticeHacia(0, 0.0f, -2048.0f);
							datRange->visualMax->moverVerticeHacia(1, 0.0f, 2048.0f + sizeHD.alto);
							datRange->visualMax->establecerTraslacion(posHdEnd, 0.0f);
						}
					}
				}
				posHd = posHdEnd;
			}
		}
	}
}

//

void AUEditorMesh::tickAnimacion(float segsTranscurridos){
	//Animate preview
	if(_previewSprite != NULL){
		AUTextura* tex = _previewSprite->textura();
		if(tex != NULL){
			const NBTamano szHD = tex->tamanoHD();
			NBRectangulo limts = _previewSprite->limites(); NBASSERT(limts.ancho > 0.0f && limts.alto < 0.0f)
			if(_meshMap->cortesEnSentido(ENMallaCorteSentido_Vertical)->conteo > 1){
				if(_previewGrowingW){
					limts.ancho += (segsTranscurridos * szHD.ancho * 0.3f);
					if(limts.ancho > (szHD.ancho * 2.0f)){
						limts.ancho = (szHD.ancho * 2.0f);
						_previewGrowingW = !_previewGrowingW;
					}
				} else {
					limts.ancho -= (segsTranscurridos * szHD.ancho * 0.3f);
					if(limts.ancho < (szHD.ancho * 0.1f)){
						limts.ancho = (szHD.ancho * 0.1f);
						_previewGrowingW = !_previewGrowingW;
					}
				}
			}
			if(_meshMap->cortesEnSentido(ENMallaCorteSentido_Horizontal)->conteo > 1){
				if(_previewGrowingH){
					limts.alto -= (segsTranscurridos * szHD.alto * 0.3f); //"-" Inverted-y
					if(limts.alto < (szHD.alto * -2.0f)){ //"<" Inverted-y
						limts.alto = (szHD.alto * -2.0f);
						_previewGrowingH = !_previewGrowingH;
					}
				} else {
					limts.alto += (segsTranscurridos * szHD.alto * 0.3f); //"+" Inverted-y
					if(limts.alto > (szHD.alto * -0.1f)){ //">" Inverted-y
						limts.alto = (szHD.alto * -0.1f);
						_previewGrowingH = !_previewGrowingH;
					}
				}
			}
			_previewSprite->redimensionar(limts.x, limts.y, limts.ancho, limts.alto);
			_previewSpriteDbg->redimensionar(limts.x, limts.y, limts.ancho, limts.alto);
		}
	}
}

//TOUCHES
void AUEditorMesh::touchIniciado(STGTouch* touch, const NBPunto &posTouchEscena, AUEscenaObjeto* objeto){
	//
}

void AUEditorMesh::touchMovido(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	if(objeto == _layerDrag){
		_workspaceCenter.x -= (posActualEscena.x - posAnteriorEscena.x);
		_workspaceCenter.y -= (posActualEscena.y - posAnteriorEscena.y);
		//Define touch as drag (not selection)
		if(touch->marcaConsumidor != ENEditorMeshTouchType_Drag){
			const NBPunto posIniPuerto = NBGestorEscena::coordenadaGrupoAPuerto(_iEscena, ENGestorEscenaGrupo_Escena, posInicialEscena.x, posInicialEscena.y);
			const NBPunto posActPuerto = NBGestorEscena::coordenadaGrupoAPuerto(_iEscena, ENGestorEscenaGrupo_Escena, posActualEscena.x, posActualEscena.y);
			if(NBPUNTO_DISTANCIA(posIniPuerto.x, posIniPuerto.y, posActPuerto.x, posActPuerto.y) > 4){
				touch->marcaConsumidor = ENEditorMeshTouchType_Drag;
				PRINTF_INFO("Touch marked as DRAG!\n");
			}
		}
		//
		this->privOrganizarContenido();
	} else {
		NBASSERT(_meshMap->cortesEnSentido((ENMallaCorteSentido)ENEditorMeshGrid_CutH)->conteo == _meshRanges[ENEditorMeshGrid_CutH]->conteo)
		NBASSERT(_meshMap->cortesEnSentido((ENMallaCorteSentido)ENEditorMeshGrid_CutV)->conteo == _meshRanges[ENEditorMeshGrid_CutV]->conteo)
		//Analyze vertical drag
		const SI32 indexDragged				= this->privIndexOfRangeWithVisualObject(objeto, ENEditorMeshGrid_CutH);
		if(indexDragged != -1){
			const SI32 posYMutiplied		= ((SI32)posActualEscena.y / _meshCoordMutliplier) * _meshCoordMutliplier;
			STMallaCorte* cut				= _meshMap->corteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutH, indexDragged); NBASSERT(cut != NULL)
			const SI32 cutStart				= (SI32) _meshMap->inicioDeCorteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutH, indexDragged);
			STEditorMeshRange* range				= _meshRanges[ENEditorMeshGrid_CutH]->elemPtr(indexDragged);
			if(objeto == range->visualMin && indexDragged > 0){
				STMallaCorte* prevCut		= _meshMap->corteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutH, indexDragged - 1); NBASSERT(prevCut != NULL)
				const SI32 prevCutStart		= (SI32) _meshMap->inicioDeCorteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutH, indexDragged - 1);
				SI32 newSizePrev			= (posYMutiplied - prevCutStart);
				if(newSizePrev < 0){ newSizePrev = 0; }
				if((prevCutStart + newSizePrev) > (cutStart + (SI32)cut->tamano)) { newSizePrev = (cutStart + (SI32)cut->tamano) - prevCutStart; }
				cut->tamano					= ((SI32)prevCut->tamano + (SI32)cut->tamano - newSizePrev);
				prevCut->tamano				= newSizePrev;
				this->privUpdateRangesRepresentations(true);
			} else if(objeto == range->visualMax && (indexDragged + 1) < _meshRanges[ENEditorMeshGrid_CutH]->conteo){
				STMallaCorte* nextCut		= _meshMap->corteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutH, indexDragged + 1); NBASSERT(nextCut != NULL)
				const SI32 nextCutStart		= (SI32)_meshMap->inicioDeCorteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutH, indexDragged + 1);
				SI32 newSizeRange			= (posYMutiplied - cutStart);
				if(newSizeRange < 0){ newSizeRange = 0; }
				if((cutStart + newSizeRange) > (nextCutStart + (SI32)nextCut->tamano)) { newSizeRange = (nextCutStart + (SI32)nextCut->tamano) - cutStart; }
				nextCut->tamano				= ((SI32)cut->tamano + (SI32)nextCut->tamano - newSizeRange);
				cut->tamano					= newSizeRange;
				this->privUpdateRangesRepresentations(true);
			}
		} else {
			//Analyze horizontal drag
			const SI32 indexDragged = this->privIndexOfRangeWithVisualObject(objeto, ENEditorMeshGrid_CutV);
			if(indexDragged != -1){
				const SI32 posXMutiplied		= ((SI32)posActualEscena.x / _meshCoordMutliplier) * _meshCoordMutliplier;
				STMallaCorte* cut				= _meshMap->corteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutV, indexDragged); NBASSERT(cut != NULL)
				const SI32 cutStart				= (SI32) _meshMap->inicioDeCorteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutV, indexDragged);
				STEditorMeshRange* range				= _meshRanges[ENEditorMeshGrid_CutV]->elemPtr(indexDragged);
				if(objeto == range->visualMin && indexDragged > 0){
					STMallaCorte* prevCut		= _meshMap->corteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutV, indexDragged - 1); NBASSERT(prevCut != NULL)
					const SI32 prevCutStart		= (SI32) _meshMap->inicioDeCorteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutV, indexDragged - 1);
					SI32 newSizePrev			= (posXMutiplied - prevCutStart);
					if(newSizePrev < 0){ newSizePrev = 0; }
					if((prevCutStart + newSizePrev) > (cutStart + (SI32)cut->tamano)) { newSizePrev = (cutStart + (SI32)cut->tamano) - prevCutStart; }
					cut->tamano		= ((SI32)prevCut->tamano + (SI32)cut->tamano - newSizePrev);
					prevCut->tamano	= newSizePrev;
					this->privUpdateRangesRepresentations(true);
				} else if(objeto == range->visualMax && (indexDragged + 1) < _meshRanges[ENEditorMeshGrid_CutV]->conteo){
					STMallaCorte* nextCut		= _meshMap->corteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutV, indexDragged + 1); NBASSERT(nextCut != NULL)
					const SI32 nextCutStart		= (SI32) _meshMap->inicioDeCorteEnIndice((ENMallaCorteSentido)ENEditorMeshGrid_CutV, indexDragged + 1);
					SI32 newSizeRange			= (posXMutiplied - cutStart);
					if(newSizeRange < 0){ newSizeRange = 0; }
					if((cutStart + newSizeRange) > (nextCutStart + (SI32)nextCut->tamano)) { newSizeRange = (nextCutStart + (SI32)nextCut->tamano) - cutStart; }
					nextCut->tamano	= ((SI32)cut->tamano + (SI32)nextCut->tamano - newSizeRange);
					cut->tamano		= newSizeRange;
					this->privUpdateRangesRepresentations(true);
				}
			}
		}
	}
}

void AUEditorMesh::touchFinalizado(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	/*const NBCajaAABB cajaEscena = objeto->cajaAABBEnEscena();
	if(NBCAJAAABB_INTERSECTA_PUNTO(cajaEscena, posActualEscena.x, posActualEscena.y)){
		//
	}*/
	if(_layerDrag == objeto){
		//Range selection
		bool consumed = false;
		//Workspace-pan or range-resize
		if(_meshTexture != NULL && !consumed && _layerEditor->idEscena != -1 && touch->marcaConsumidor != ENEditorMeshTouchType_Drag){
			const NBTamano sizeHD = _meshTexture->tamanoHD();
			//Click over a vertical-range?
			if((posActualEscena.y < 0.0f || posActualEscena.y > sizeHD.alto) && posActualEscena.x >= 0.0f && posActualEscena.x < sizeHD.ancho){
				SI32 j; const SI32 rangesCount = _meshRanges[ENEditorMeshGrid_CutV]->conteo;
				for(j = 0; j < rangesCount; j++){
					STEditorMeshRange* datRange = _meshRanges[ENEditorMeshGrid_CutV]->elemPtr(j);
					if(datRange->visualRange != NULL){
						AUEscenaFigura* fig = datRange->visualRange;
						const NBCajaAABB cajaEscena = fig->cajaAABBEnEscena();
						if(posActualEscena.x >= cajaEscena.xMin && posActualEscena.x <= cajaEscena.xMax){
							this->privSelectionSet(ENEditorMeshGrid_CutV, j);
							this->privUpdateRangesRepresentations(false);
							consumed = true;
							break;
						}
					}
				}
			}
			//Click over a horizontal-range?
			if((posActualEscena.x < 0.0f || posActualEscena.x > sizeHD.ancho) && posActualEscena.y >= 0.0f && posActualEscena.y < sizeHD.alto){
				SI32 j; const SI32 rangesCount = _meshRanges[ENEditorMeshGrid_CutH]->conteo;
				for(j = 0; j < rangesCount; j++){
					STEditorMeshRange* datRange = _meshRanges[ENEditorMeshGrid_CutH]->elemPtr(j);
					if(datRange->visualRange != NULL){
						AUEscenaFigura* fig = datRange->visualRange;
						const NBCajaAABB cajaEscena = fig->cajaAABBEnEscena();
						if(posActualEscena.y >= cajaEscena.yMin && posActualEscena.y <= cajaEscena.yMax){
							this->privSelectionSet(ENEditorMeshGrid_CutH, j);
							this->privUpdateRangesRepresentations(false);
							consumed = true;
							break;
						}
					}
				}
			}
		}
		//Disable range-selection
		if(!consumed && touch->marcaConsumidor != ENEditorMeshTouchType_Drag){
			this->privSelectionSet(ENEditorMeshGrid_Count, -1);
			this->privUpdateRangesRepresentations(false);
		}
	}
	//Finalize limit-drag (remove empty ranges)
	if(objeto != _layerDrag){
		//Analyze vertical drag
		const SI32 indexDragged = this->privIndexOfRangeWithVisualObject(objeto, ENEditorMeshGrid_CutH);
		if(indexDragged != -1){
			STEditorMeshRange* range = _meshRanges[ENEditorMeshGrid_CutH]->elemPtr(indexDragged);
			if(objeto == range->visualMin && indexDragged > 0){
				this->privRemoveEmptyRanges();
			} else if(objeto == range->visualMax && (indexDragged + 1) < _meshRanges[ENEditorMeshGrid_CutH]->conteo){
				this->privRemoveEmptyRanges();
			}
		} else {
			//Analyze horizontal drag
			const SI32 indexDragged = this->privIndexOfRangeWithVisualObject(objeto, ENEditorMeshGrid_CutV);
			if(indexDragged != -1){
				STEditorMeshRange* range = _meshRanges[ENEditorMeshGrid_CutV]->elemPtr(indexDragged);
				if(objeto == range->visualMin && indexDragged > 0){
					this->privRemoveEmptyRanges();
				} else if(objeto == range->visualMax && (indexDragged + 1) < _meshRanges[ENEditorMeshGrid_CutV]->conteo){
					this->privRemoveEmptyRanges();
				}
			}
		}
	}
}

//HOVER
void AUEditorMesh::hoverIniciado(AUEscenaObjeto* objeto){
	//PRINTF_INFO("Hover iniciado sobre %s.\n", objeto->nombreUltimaClase());
	if(_selMeshRangeIndex >= 0){
		STEditorMeshRange* range = _meshRanges[_selMeshRangeOrientation]->elemPtr(_selMeshRangeIndex);
		if(range->visualMin == objeto){
			_selMeshRangeHoverMask = RANGE_MASK_ITEM_MIN;
			this->privUpdateRangesRepresentations(false);
		} else if(range->visualMax == objeto){
			_selMeshRangeHoverMask = RANGE_MASK_ITEM_MAX;
			this->privUpdateRangesRepresentations(false);
		} else if(range->visualRange == objeto){
			_selMeshRangeHoverMask = RANGE_MASK_ITEM_CENTER;
			this->privUpdateRangesRepresentations(false);
		}
	}
}

void AUEditorMesh::hoverMovido(const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	//PRINTF_INFO("Hover movido sobre %s.\n", objeto->nombreUltimaClase());
}

void AUEditorMesh::hoverFinalizado(AUEscenaObjeto* objeto){
	//PRINTF_INFO("Hover finalizado sobre %s.\n", objeto->nombreUltimaClase());
	if(_selMeshRangeIndex >= 0){
		STEditorMeshRange* range = _meshRanges[_selMeshRangeOrientation]->elemPtr(_selMeshRangeIndex);
		if(range->visualMin == objeto){
			_selMeshRangeHoverMask &= ~RANGE_MASK_ITEM_MIN;
			this->privUpdateRangesRepresentations(false);
		} else if(range->visualMax == objeto){
			_selMeshRangeHoverMask &= ~RANGE_MASK_ITEM_MAX;
			this->privUpdateRangesRepresentations(false);
		} else if(range->visualRange == objeto){
			_selMeshRangeHoverMask &= ~RANGE_MASK_ITEM_CENTER;
			this->privUpdateRangesRepresentations(false);
		}
	}
}

//TECLAS (old)
bool AUEditorMesh::teclaPresionada(SI32 codigoTecla){
	return false;
}

bool AUEditorMesh::teclaLevantada(SI32 codigoTecla){
	return false;
}

bool AUEditorMesh::teclaEspecialPresionada(SI32 codigoTecla){
	return false;
}

//TECLAS

void AUEditorMesh::textEditorCloneInterfaceObjectsRetained(AUObjeto** dstVisualObj, ITextInputListener** dstVisualItf, AUObjeto** dstValueObj, ITextInputListener** dstValueItf){
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

STTextEditorSyncResult AUEditorMesh::textEditorCompareClones(AUObjeto* visualClone, AUObjeto* valueClone){
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

STTextEditorSyncResult AUEditorMesh::textEditorSyncClones(AUObjeto* visualClone, AUObjeto* valueClone){
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

void AUEditorMesh::textEditorFocusGained(){
	//
}

void AUEditorMesh::textEditorFocusLost(const ENNBKeybFocusLostType actionType){
	//
}

bool AUEditorMesh::textEditorMantainBeforeTouchConsumer(AUEscenaObjeto* touchConsumer){
	return true;
}

//

void AUEditorMesh::teclaFocoObtenido(){
	//
}

void AUEditorMesh::teclaFocoPerdido(){
	//
}

bool AUEditorMesh::teclaMantenerFocoAnteConsumidorDeTouch(AUEscenaObjeto* consumidorTouch){
	return true;
}

bool AUEditorMesh::teclaTieneTexto(){
	return false;
}

void AUEditorMesh::teclaBackspace(){
	//
}

void AUEditorMesh::teclaIntroducirTexto(const char* textoIntroducir){
	bool consumed = false;
	switch (textoIntroducir[0]) {
		case '+':
			//Add new range
			if(_selMeshRangeIndex >= 0){
				STMallaCorte* selCut	= _meshMap->corteEnIndice((ENMallaCorteSentido)_selMeshRangeOrientation, _selMeshRangeIndex);
				_meshMap->agregarCorteEnIndice((ENMallaCorteSentido)_selMeshRangeOrientation, _selMeshRangeIndex + 1, selCut->tamano / 2, ENMallaCorteTipo_Rigido);
				selCut->tamano			/= 2;
				this->privUpdateRangesRepresentations(true);
				consumed = true;
			}
			//Scale
			if(!consumed){
				this->privWorkspaceSetScale(_workspaceScale * 1.1f);
			}
			break;
		case '-':
			//Remove range
			if(_selMeshRangeIndex >= 0){
				if((_selMeshRangeHoverMask & ~RANGE_MASK_ITEM_MIN) != 0){
					//Remove selected range and grow previous-one
					if(this->privRemoveRangeAtIndex(_selMeshRangeOrientation, _selMeshRangeIndex, false)){
						this->privSelectionSet(ENEditorMeshGrid_Count, -1);
						this->privUpdateRangesRepresentations(true);
					}
					consumed = true;
				} else if((_selMeshRangeHoverMask & ~RANGE_MASK_ITEM_MAX) != 0){
					//Remove selected rage and grow next-one
					if(this->privRemoveRangeAtIndex(_selMeshRangeOrientation, _selMeshRangeIndex, true)){
						this->privSelectionSet(ENEditorMeshGrid_Count, -1);
						this->privUpdateRangesRepresentations(true);
					}
					consumed = true;
				}
				
			}
			//Scale
			if(!consumed){
				this->privWorkspaceSetScale(_workspaceScale * 0.9f);
			}
			break;
		case 't':
		case 'T':
			//Change type of range
			if(_selMeshRangeIndex >= 0 && _meshMap != NULL){
				STMallaCorte* selCut	= _meshMap->corteEnIndice((ENMallaCorteSentido)_selMeshRangeOrientation, _selMeshRangeIndex);
				_meshMap->establecerCorteTipoEnIndice((ENMallaCorteSentido)_selMeshRangeOrientation, _selMeshRangeIndex, (selCut->tipo == ENMallaCorteTipo_Rigido ? ENMallaCorteTipo_Estirar : selCut->tipo == ENMallaCorteTipo_Estirar ? ENMallaCorteTipo_Repetir : ENMallaCorteTipo_Rigido));
				this->privUpdateRangesRepresentations(false);
			}
			break;
		case 'g':
		case 'G':
			//Save
			if(!consumed){
				_guiMenu->optionAnimate(ENEditorMeshButton_Save, this->privMeshSaveAction());
			}
			break;
		case 'p':
		case 'P':
			if(_previewSprite != NULL){
				AUEscenaContenedor* parent;
				//Restore editor
				parent = (AUEscenaContenedor*)_previewSprite->contenedor();
				if(parent != NULL) parent->quitarObjetoEscena(_previewSprite);
				_previewSprite->liberar();
				_previewSprite = NULL;
				//
				parent = (AUEscenaContenedor*)_previewSpriteDbg->contenedor();
				if(parent != NULL) parent->quitarObjetoEscena(_previewSpriteDbg);
				_previewSpriteDbg->liberar();
				_previewSpriteDbg = NULL;
				//
				parent = (AUEscenaContenedor*)_layerPreview->contenedor();
				if(parent != NULL) parent->quitarObjetoEscena(_layerPreview);
				//
				_layerMain->agregarObjetoEscena(_layerEditor);
			} else {
				AUEscenaContenedor* parent;
				//Set preview
				_previewGrowingW = _previewGrowingH = true;
				//
				_previewSprite	= new(this) AUEscenaSpriteElastico(_meshTexture);
				_previewSprite->establecerMallaMapa(_meshMap);
				_layerPreview->agregarObjetoEscena(_previewSprite);
				//
				_previewSpriteDbg	= new(this) AUEscenaSpriteElastico(_meshTexture);
				_previewSpriteDbg->establecerMultiplicadorColor8(0, 0, 255, 100);
				//_layerPreview->agregarObjetoEscena(_previewSpriteDbg);
				//
				parent = (AUEscenaContenedor*)_layerEditor->contenedor();
				if(parent != NULL) parent->quitarObjetoEscena(_layerEditor);
				//
				_layerMain->agregarObjetoEscena(_layerPreview);
			}
			break;
		default:
			break;
	}
}

//

SI32 AUEditorMesh::privRemoveEmptyRanges(){
	NBASSERT(_meshMap->cortesEnSentido((ENMallaCorteSentido)ENEditorMeshGrid_CutH)->conteo == _meshRanges[ENEditorMeshGrid_CutH]->conteo)
	NBASSERT(_meshMap->cortesEnSentido((ENMallaCorteSentido)ENEditorMeshGrid_CutV)->conteo == _meshRanges[ENEditorMeshGrid_CutV]->conteo)
	//
	SI32 i, j, countRemoved = 0; bool selectionDisabled = false;
	for(i = 0; i < ENMallaCorteSentido_Conteo; i++){
		const AUArregloNativoP<STMallaCorte>* cortes = _meshMap->cortesEnSentido((ENMallaCorteSentido)i);
		const SI32 count = cortes->conteo;
		for(j = (count - 1); j >= 0; j--){
			STMallaCorte* datcut	= cortes->elemPtr(j);
			if(datcut->tamano <= 0){
				if(_selMeshRangeOrientation == i && _selMeshRangeIndex == j){
					this->privSelectionSet(ENEditorMeshGrid_Count, -1);
					selectionDisabled = true;
				}
				_meshMap->quitarCorteEnIndice((ENMallaCorteSentido)i, j);
				countRemoved++;
			}
		}
	}
	if(countRemoved > 0 || selectionDisabled){
		this->privUpdateRangesRepresentations(true);
	}
	return countRemoved;
}

bool AUEditorMesh::privRemoveRangeAtIndex(const ENEditorMeshGrid orientation, const SI32 index, const bool addSizeToNext){
	bool r = false;
	if(index >= 0){
		if(!addSizeToNext && index > 0){
			//Add to previous
			STMallaCorte* selCut	= _meshMap->corteEnIndice((ENMallaCorteSentido)orientation, index);
			STMallaCorte* prevCut	= _meshMap->corteEnIndice((ENMallaCorteSentido)orientation, index - 1);
			prevCut->tamano			+= selCut->tamano;
			r = true;
		} else if((index + 1) < _meshMap->cortesEnSentido((ENMallaCorteSentido)orientation)->conteo){
			//Add to next
			STMallaCorte* selCut	= _meshMap->corteEnIndice((ENMallaCorteSentido)orientation, index);
			STMallaCorte* nextCut	= _meshMap->corteEnIndice((ENMallaCorteSentido)orientation, index + 1);
			nextCut->tamano			+= selCut->tamano;
			r = true;
		}
	}
	return r;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUEditorMesh)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUEditorMesh, "AUEditorMesh")
AUOBJMETODOS_CLONAR_NULL(AUEditorMesh)

