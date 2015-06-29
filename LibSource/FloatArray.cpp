#include "FloatArray.h"
#include "basicmaths.h"
#include "message.h"
#include <string.h>

void FloatArray::getMin(float* value, int* index){
#ifdef ARM_CORTEX
  unsigned long idx;
  arm_min_f32(data, size, value, &idx);
  *index = (int)idx;
#else
  *value=data[0];
  *index=0;
  for(int n=1; n<size; n++){
    float currentValue=data[n];
    if(currentValue<*value){
      *value=currentValue;
      *index=n;
    }
  }
#endif
}

float FloatArray::getMinValue(){
  float value;
  int index;
  getMin(&value, &index);
  return value;
}

int FloatArray::getMinIndex(){
  float value;
  int index;
  getMin(&value, &index);
  return index;
}

void FloatArray::getMax(float* value, int* index){
#ifdef ARM_CORTEX  
  unsigned long idx;
  arm_max_f32(data, size, value, &idx);
  *index = (int)idx;
#else
  *value=data[0];
  *index=0;
  for(int n=1; n<size; n++){
    float currentValue=data[n];
    if(currentValue>*value){
      *value=currentValue;
      *index=n;
    }
  }
#endif
}

float FloatArray::getMaxValue(){
  float value;
  int index;
  getMax(&value, &index);
  return value;
}

int FloatArray::getMaxIndex(){
  float value;
  int index;
  getMax(&value, &index);
  return index;
}

void FloatArray::rectify(FloatArray& destination){ //this is actually "copy data with rectifify"
  int minSize= min(size,destination.getSize()); //TODO: shall we take this out and allow it to segfault?
#ifdef ARM_CORTEX  
  arm_abs_f32( (float*)data, (float*)destination, size);
#else
  for(int n=0; n<minSize; n++){
    destination[n]=abs(data[n]);
  }
#endif  
}

void FloatArray::rectify(){//in place
  rectify(*this);
}

void FloatArray::reverse(FloatArray& destination){ //this is actually "copy data with reverse"
  if(destination==*this){ //make sure it is not called "in-place"
    reverse();
    return;
  }
  for(int n=0; n<size; n++){
    destination[n]=data[size-n-1];
  }
}

void FloatArray::reverse(){//in place
  for(int n=0; n<size/2; n++){
    float temp=data[n];
    data[n]=data[size-n-1];
    data[size-n-1]=temp;
  }
}

void FloatArray::reciprocal(FloatArray& destination){
  float* data = getData();
  for(int n=0; n<getSize(); n++)
    destination[n] = 1/data[n];
}

void FloatArray::reciprocal(){//in place
  reciprocal(*this);
}

float FloatArray::getRms(){
  float result;
#ifdef ARM_CORTEX  
  arm_rms_f32 (data, size, &result);
#else
  result=0;
  float *pSrc= data;
  for(int n=0; n<size; n++){
    result += pSrc[n]*pSrc[n];
  }
  result=sqrtf(result/size);
#endif
  return result;
}

float FloatArray::getMean(){
  float result;
#ifdef ARM_CORTEX  
  arm_mean_f32 (data, size, &result);
#else
  result=0;
  for(int n=0; n<size; n++){
    result+=data[n];
  }
  result=result/size;
#endif
  return result;
}

float FloatArray::getPower(){
  float result;
#ifdef ARM_CORTEX  
  arm_power_f32 (data, size, &result);
#else
  result=0;
  float *pSrc = data;
  for(int n=0; n<size; n++){
    result += pSrc[n]*pSrc[n];
  }
#endif
  return result;
}

float FloatArray::getStandardDeviation(){
  float result;
#ifdef ARM_CORTEX  
  arm_std_f32 (data, size, &result);
#else
  result=sqrtf(getVariance());
#endif
  return result;
}

float FloatArray::getVariance(){
  float result;
#ifdef ARM_CORTEX  
  arm_var_f32(data, size, &result);
#else
  float sumOfSquares=getPower();
  float sum=0;
  for(int n=0; n<size; n++){
    sum+=data[n];
  }
  result=(sumOfSquares - sum*sum/size) / (size - 1);
#endif
  return result;
}
void FloatArray::scale(float factor, FloatArray destination){//supports in-place
#ifdef ARM_CORTEX  
  arm_scale_f32(data, factor, destination, size);
#else
  for(int n=0; n<size; n++){
    destination[n]=factor*data[n];
  }
#endif
}

void FloatArray::scale(float factor){
  scale(factor, *this);
}

FloatArray FloatArray::subarray(int offset, int length){
  ASSERT(size >= offset+length, "Array too small");
  return FloatArray(data+offset, length);
}

void FloatArray::copyTo(FloatArray destination){
  copyTo(destination, min(size, destination.getSize()));
}

void FloatArray::copyFrom(FloatArray source){
  copyFrom(source, min(size, source.getSize()));
}

void FloatArray::copyTo(float* other, int length){
  ASSERT(size >= length, "Array too small");
  #ifdef ARM_CORTEX
  arm_copy_f32(data, other, length);
  #endif /* ARM_CORTEX */
  // memcpy(other.data, data, size*sizeof(float));
}

void FloatArray::copyFrom(float* other, int length){
  ASSERT(size >= length, "Array too small");
  #ifdef ARM_CORTEX
  arm_copy_f32(other, data, length);
  #endif /* ARM_CORTEX */
}

void FloatArray::insert(FloatArray source, int offset, int samples){
  ASSERT(size >= offset+samples, "Array too small");
  #ifdef ARM_CORTEX
  arm_copy_f32(source.data, data+offset, samples);  
  #endif /* ARM_CORTEX */
}

void FloatArray::move(int fromIndex, int toIndex, int samples){
  ASSERT(size >= toIndex+samples, "Array too small");
  memmove(data+toIndex, data+fromIndex, samples*sizeof(float));
}

void FloatArray::setAll(float value){
  #ifdef ARM_CORTEX
  arm_fill_f32(value, data, size);
  #endif /* ARM_CORTEX */
}

void FloatArray::add(FloatArray operand2, FloatArray destination){ //allows in-place
  ASSERT(operand2.size == size &&  destination.size==size, "Arrays must be same size");
  #ifdef ARM_CORTEX
  /* despite not explicitely documented in the CMSIS documentation,
      this has been tested to behave properly even when pSrcA==pDst
      void 	arm_add_f32 (float32_t *pSrcA, float32_t *pSrcB, float32_t *pDst, uint32_t blockSize)
  */
  arm_add_f32(data, operand2.data, destination.data, size); 
  #endif /* ARM_CORTEX */
}

void FloatArray::add(FloatArray operand2){ //in-place
  add(operand2, *this);
}

void FloatArray::add(float scalar){
  for(int n=0; n<size; n++){
   data[n]+=scalar;
  } 
}

void FloatArray::subtract(FloatArray operand2, FloatArray destination){ //allows in-place
  ASSERT(operand2.size == size && destination.size==size, "Arrays must be same size");
  #ifdef ARM_CORTEX
  /* despite not explicitely documented in the CMSIS documentation,
      this has been tested to behave properly even when pSrcA==pDst
      void 	arm_sub_f32 (float32_t *pSrcA, float32_t *pSrcB, float32_t *pDst, uint32_t blockSize)
  */
  arm_sub_f32(data, operand2.data, destination.data, size);
  #endif /* ARM_CORTEX */
}

void FloatArray::subtract(FloatArray operand2){ //in-place
  subtract(operand2, *this);
}

void FloatArray::subtract(float scalar){
  for(int n=0; n<size; n++){
    data[n]-=scalar;
  } 
}

void FloatArray::multiply(FloatArray operand2, FloatArray destination){ //allows in-place
  ASSERT(operand2.size == size &&  destination.size==size, "Arrays must be same size");
  #ifdef ARM_CORTEX
  /* despite not explicitely documented in the CMSIS documentation,
      this has been tested to behave properly even when pSrcA==pDst
      void 	arm_mult_f32 (float32_t *pSrcA, float32_t *pSrcB, float32_t *pDst, uint32_t blockSize)
  */
    arm_mult_f32(data, operand2.data, destination, size);
  #endif /* ARM_CORTEX */
}

void FloatArray::multiply(FloatArray operand2){ //in-place
  multiply(operand2, *this);
}

void FloatArray::multiply(float scalar){
  for(int n=0; n<size; n++){
   data[n]*=scalar;
  } 
}

void FloatArray::negate(FloatArray& destination){//allows in-place
  #ifdef ARM_CORTEX
  arm_negate_f32(data, destination.getData(), size); 
  #endif /* ARM_CORTEX */
}
void FloatArray::negate(){
  negate(*this);
}

void FloatArray::noise(){
  for(int n=0; n<size; n++){
    data[n]=rand()/(float)RAND_MAX *2 -1;
  }
}
/**
 * Perform the convolution of this FloatArray with @other, putting the result in @destination.
 * @destination must have a minimum size of this+other-1.
 */
void FloatArray::convolve(FloatArray other, FloatArray destination){
  ASSERT(destination.size >= size + other.size -1, "Destination array too small");
#ifdef ARM_CORTEX
  arm_conv_f32(data, size, other.data, other.size, destination);
#endif /* ARM_CORTEX */
}

/**
 * Perform partial convolution: start at @offset and compute @samples points
 */
void FloatArray::convolve(FloatArray other, FloatArray destination, int offset, int samples){
  ASSERT(destination.size >= samples, "Destination array too small");
#ifdef ARM_CORTEX
  arm_conv_partial_f32(data, size, other.data, other.size, destination, offset, samples);
#endif /* ARM_CORTEX */
}

/*
 * @destination must have a minimum size of 2*max(srcALen, srcBLen)-1.
 */
void FloatArray::correlate(FloatArray other, FloatArray destination){
  ASSERT(destination.size >= 2 * max(size, other.size)-1, "Destination array too small");
#ifdef ARM_CORTEX
  arm_correlate_f32(data, size, other.data, other.size, destination);
#endif /* ARM_CORTEX */
}

FloatArray FloatArray::create(int size){
  return FloatArray(new float[size], size);
}

void FloatArray::destroy(FloatArray array){
  delete array.data;
}
