//
//  NBAtlasTools.cpp
//  app-refranero
//
//  Created by Marcos Ortega on 14/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#include "NoteuCorePrecomp.h"
#include "NBAtlasTools.h"
//

bool NBAtlasTools::fixMasksBorders(AUMapaBits* bitmap, AUAtlasMapa* atlasMap, const SI32 maxAbsDeltaInColorComponents, const AUArregloNativoP<SI32>* inOptIndexes){
	bool r = false;
	NBTamanoP<UI16> szAtlas = atlasMap->tamano();
	MapaDeBitsDesciptor bmpProps = bitmap->propiedades();
	if(szAtlas.ancho == bmpProps.ancho && szAtlas.alto == bmpProps.alto){
		BYTE* data = bitmap->datos();
		const MapaDeBitsDesciptor props = bitmap->propiedades(); NBASSERT(props.color == COLOR_RGBA8)
		const SI32 lines = props.alto;
		//
		const AUArregloNativoP<STAtlasArea>* areas = atlasMap->areasOcupadas();
		SI32 i; const SI32 count = areas->conteo;
		for(i = 0; i < count; i++){
			bool process = true;
			if(inOptIndexes != NULL){
				process = (inOptIndexes->indiceDe(i) != -1);
			}
			if(process){
				const NBRectanguloUI16 area = areas->elemPtr(i)->area;
				SI32 y; const SI32 areaNextY = area.y + area.alto;
				//PRINTF_INFO("Analyzing area #%d: (%d, %d) - (%d, %d).\n", (i+1), area.x, area.y, area.ancho, area.alto);
				//
				bool isMask = true;
				//Analyze colors
				{
					SI32 diffRG, diffRB, diffRA;
					for(y = area.y; y < areaNextY && y < lines && isMask; y++){
						BYTE* posAtLine = &data[((SI32)props.bytesPorLinea * y) + (area.x * (props.bitsPorPixel / 8))];
						const BYTE* posAtLineNextEnd = &data[((SI32)props.bytesPorLinea * y) + ((area.x + area.ancho) * (props.bitsPorPixel / 8))];
						const BYTE* posAtLineNext = &data[(SI32)props.bytesPorLinea * (y + 1)];
						while(posAtLine < posAtLineNext && posAtLine < posAtLineNextEnd){
							if(posAtLine[3] > 0 && posAtLine[3] < 255){
								diffRG = ((SI32)posAtLine[0] - (SI32)posAtLine[1]);
								diffRB = ((SI32)posAtLine[0] - (SI32)posAtLine[2]);
								diffRA = ((SI32)posAtLine[0] - (SI32)posAtLine[3]);
								//PRINTF_INFO("Deltas pix(%d, %d, %d, %d), RG(%d), RB(%d), RA(%d).\n", posAtLine[0], posAtLine[1], posAtLine[2], posAtLine[3], diffRG, diffRB, diffRA);
								if((diffRG < -maxAbsDeltaInColorComponents || diffRG > maxAbsDeltaInColorComponents) || (diffRB < -maxAbsDeltaInColorComponents || diffRB > maxAbsDeltaInColorComponents) || (diffRA < -maxAbsDeltaInColorComponents || diffRA > maxAbsDeltaInColorComponents)){
									isMask = false;
									break;
								}
							}
							posAtLine += 4;
						}
					}
				}
				/*if(isMask){
				 PRINTF_INFO("IS-MASK.\n");
				 } else {
				 PRINTF_INFO("IS-NOT-MASK.\n");
				 }*/
				//Fix colors to avoid black border arround white shapes
				if(isMask){
					for(y = area.y; y < areaNextY && y < lines && isMask; y++){
						BYTE* posAtLine = &data[((SI32)props.bytesPorLinea * y) + (area.x * (props.bitsPorPixel / 8))];
						const BYTE* posAtLineNextEnd = &data[((SI32)props.bytesPorLinea * y) + ((area.x + area.ancho) * (props.bitsPorPixel / 8))];
						const BYTE* posAtLineNext = &data[(SI32)props.bytesPorLinea * (y + 1)];
						while(posAtLine < posAtLineNext && posAtLine < posAtLineNextEnd){
							if(posAtLine[3] > 0 && posAtLine[3] < 255){
								posAtLine[0] = posAtLine[1] = posAtLine[2] = 255;
							}
							posAtLine += 4;
						}
					}
				}
			}
		}
		r = true;
	}
	return r;
}


bool NBAtlasTools::invertColors(AUMapaBits* bitmap, AUAtlasMapa* atlasMap, const AUArregloNativoP<SI32>* inOptIndexes){
	bool r = false;
	NBTamanoP<UI16> szAtlas = atlasMap->tamano();
	MapaDeBitsDesciptor bmpProps = bitmap->propiedades();
	if(szAtlas.ancho == bmpProps.ancho && szAtlas.alto == bmpProps.alto){
		BYTE* data = bitmap->datos();
		const MapaDeBitsDesciptor props = bitmap->propiedades(); NBASSERT(props.color == COLOR_RGBA8)
		const SI32 lines = props.alto;
		//
		if(inOptIndexes == NULL){
			SI32 y;
			for(y = 0; y < lines; y++){
				BYTE* posAtLine = &data[((SI32)props.bytesPorLinea * y)];
				const BYTE* posAtLineNext = &data[(SI32)props.bytesPorLinea * (y + 1)];
				while(posAtLine < posAtLineNext){
					if(posAtLine[3] > 0){
						posAtLine[0] = 255 - posAtLine[0];
						posAtLine[1] = 255 - posAtLine[1];
						posAtLine[2] = 255 - posAtLine[2];
					}
					posAtLine += 4;
				}
			}
		} else {
			const AUArregloNativoP<STAtlasArea>* areas = atlasMap->areasOcupadas();
			SI32 i; const SI32 count = areas->conteo;
			for(i = 0; i < count; i++){
				if(inOptIndexes->indiceDe(i) != -1){
					const NBRectanguloUI16 area = areas->elemPtr(i)->area;
					SI32 y; const SI32 areaNextY = area.y + area.alto;
					for(y = area.y; y < areaNextY && y < lines; y++){
						BYTE* posAtLine = &data[((SI32)props.bytesPorLinea * y) + (area.x * (props.bitsPorPixel / 8))];
						const BYTE* posAtLineNextEnd = &data[((SI32)props.bytesPorLinea * y) + ((area.x + area.ancho) * (props.bitsPorPixel / 8))];
						const BYTE* posAtLineNext = &data[(SI32)props.bytesPorLinea * (y + 1)];
						while(posAtLine < posAtLineNext && posAtLine < posAtLineNextEnd){
							if(posAtLine[3] > 0){
								posAtLine[0] = 255 - posAtLine[0];
								posAtLine[1] = 255 - posAtLine[1];
								posAtLine[2] = 255 - posAtLine[2];
							}
							posAtLine += 4;
						}
					}
				}
			}
		}
		r = true;
	}
	return r;
}

