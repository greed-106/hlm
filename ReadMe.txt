The copyright in this software is being made available under the License included below.
This software may be subject to other third party and contributor rights, including patent
rights, and no such rights are granted under this license.

Copyright (C) 2021, Hangzhou Hikvision Digital Technology Co., Ltd. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted
only for the purpose of developing standards within Audio and Video Coding Standard Workgroup of
China (AVS) and for testing and promoting such standards. The following conditions are required
to be met:

  * Redistributions of source code must retain the above copyright notice, this list of
    conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other materials
    provided with the distribution.
  * The name of Hangzhou Hikvision Digital Technology Co., Ltd. may not be used to endorse or
    promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.

#***************************************************************************************************

1) Description

This package includes a test model for AVS near-lossless exploration experiment. 
The colour formats supported by the test model:

* RGB 4:4:4, YUV 4:4:4/4:2:2/4:2:0/4:0:0

The bitdepth supported by the test model:
  * 8, 10, 12, 14, 16 bits/component
  
Note: if bitdepth is larger than 8, each sample should be padded to 16-bit(pad MSBs with zeros)

#***************************************************************************************************

2) Usage

Please follow the rules below for usage of the HLM test model.

#***************************************************************************************************

2.1) Running the test model (encoder)

DemoHLMC.exe -c /data/config/encode_AI.cfg ...

Required parameters:

                 -i                 |  file address of source image
                 -o                 |  file address of compressed bitstream
                 -rec               |  file address of reconstructed image
                 -wdt               |  input image width
                 -hgt               |  input image height
                 -fr                |  frame rate
                 -fn                |  frame number
                 -bppi              |  target bpp (bits per pixel) for i-frame with precision 1/16
                 -bppp              |  target bpp (bits per pixel) for p-frame with precision 1/16
				 -I                 |  intra period
                 -bitdepth          |  bit depth
                 -format            |  RGB444 / YUV444 / YUV422 / YUV420 / YUV400 (format of source image)
				 -l                 |  file address of encoder log
                 -[other options]   |  other parameters refer to "encode_AI.cfg"

#***************************************************************************************************

2.2) Running the test model (decoder)

DemoHLMD.exe ...

Required parameters:

                 -i                 |  file address of compressed bitstream
                 -y                 |  output reconstructed image or not, usually set as 1

#***************************************************************************************************

2.3) Example usage

* Example 1:

Note: No line breaks in actual input

DemoHLMC.exe     -c                    /data/config/encode_AI.cfg
                 -i                    /data/input/DucksAndLegs_1920x1080_30hz_10bit_444p.yuv
                 -o                    /data/output/DucksAndLegs_1920x1080_30hz_10bit_444p.bin
                 -rec                  /data/output/DucksAndLegs_1920x1080_30hz_10bit_444p_rec.yuv
                 -wdt                  1920
                 -hgt                  1080
                 -fr                   30
                 -fn                   60
                 -bppi                 38
                 -bppp                 38
                 -I                    1
                 -bitdepth             10
                 -format               1
				 -l                    /data/output/enc_log.txt

DemoHLMD.exe     -i                    /data/output/DucksAndLegs_1920x1080_30hz_10bit_444p.bin
                 -y                    1

#***************************************************************************************************

3) Compressed bitstream

  * default:    2.375 bpp
  * resolution: 1/16 bpp

The compressed bitstream is coded as a binary file.
