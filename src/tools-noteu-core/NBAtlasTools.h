//
//  AUGpMenuFinPartida.h
//  game-Refranero
//
//  Created by Marcos Ortega on 14/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#ifndef NBAtlasTools_h
#define NBAtlasTools_h

class NBAtlasTools {
	public:
		static bool	fixMasksBorders(AUMapaBits* bitmap, AUAtlasMapa* atlasMap, const SI32 maxAbsDeltaInColorComponents, const AUArregloNativoP<SI32>* inOptIndexes);
		static bool invertColors(AUMapaBits* bitmap, AUAtlasMapa* atlasMap, const AUArregloNativoP<SI32>* inOptIndexes);
};

#endif
