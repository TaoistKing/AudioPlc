/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_AUDIO_CODECS_AUDIO_DECODER_H_
#define API_AUDIO_CODECS_AUDIO_DECODER_H_

namespace webrtc {

class AudioDecoder {
 public:
  enum SpeechType {
    kSpeech = 1,
    kComfortNoise = 2,
  };
};

}  // namespace webrtc
#endif  // API_AUDIO_CODECS_AUDIO_DECODER_H_
