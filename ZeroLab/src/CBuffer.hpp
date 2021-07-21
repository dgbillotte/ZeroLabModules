

struct CBuffer {
	int mBufferSize;
	int mReadIndex;
	int mWriteIndex;
	float* mBuf;

	CBuffer(int bufferSize=96000) {
		mBufferSize = bufferSize;
		mBuf = new float[mBufferSize];
		reset();
	}

	void reset() {
		memset(mBuf, 0, mBufferSize*sizeof(float));
		mReadIndex = 0;
		mWriteIndex = 0;		
	}

	void write(float input) {
		mBuf[mWriteIndex] = input;

		mWriteIndex++;
		if(mWriteIndex >= mBufferSize)
			mWriteIndex = 0;

		// mReadIndex++;
		// if(mReadIndex >= mBufferSize)
		// 	mReadIndex = 0;
	}

	// void setDelay(int samples) {
	// 	if(samples <= mWriteIndex)
	// 		mReadIndex = mWriteIndex - samples;
	// 	else
	// 		mReadIndex = mBufferSize - (samples - mWriteIndex) - 1;
	// }

	// float read() {
	// 	return mBuf[mReadIndex];
	// }

	// currently, if samples is larger than mBufferSize, it gets modded down into range
	// future, make negative values work properly

	float read_num_samples(int samples) {
		if(samples > mBufferSize)
			samples = samples % mBufferSize;

		// if(samples == 0)
		// 	samples = 1;

		if(samples <= mWriteIndex) {
			// std::cout << "case1: " << samples << "\n";
			return mBuf[mWriteIndex-(samples+1)];
		}

		// std::cout << "case2\n";
		return mBuf[mBufferSize - (samples + 1 - mWriteIndex) - 1];
	}
};