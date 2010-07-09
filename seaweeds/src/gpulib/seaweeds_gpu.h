#ifndef _SEAWEEDS_GPU_AUTO_GENERATED_H_
#define _SEAWEEDS_GPU_AUTO_GENERATED_H_

/**************************************************************************** 
                                                                              
Copyright (c) 2003, Stanford University                                       
All rights reserved.                                                          
                                                                              
Copyright (c) 2008, Advanced Micro Devices, Inc.                              
All rights reserved.                                                          
                                                                              
                                                                              
The BRCC portion of BrookGPU is derived from the cTool project                
(http://ctool.sourceforge.net) and distributed under the GNU Public License.  
                                                                              
Additionally, see LICENSE.ctool for information on redistributing the         
contents of this directory.                                                   
                                                                              
****************************************************************************/ 

#include "brook/Stream.h" 
#include "brook/KernelInterface.h" 

//! Kernel declarations
class __compnet_stage_2d
{
    public:
        void operator()(const ::brook::Stream< char >& x, const ::brook::Stream< char >& y, const int  offset, const int  len, const int  x_start, const int  y_end, const ::brook::Stream< int >& p, const ::brook::Stream<  int >& o);
        EXTENDCLASS();
};
extern __THREAD__ __compnet_stage_2d compnet_stage_2d;

#endif // _SEAWEEDS_GPU_AUTO_GENERATED_H_

