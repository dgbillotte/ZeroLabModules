//#include <stdexcept>

struct CBuffer {
	int mBufferSize;
	int mWriteIndex;
	float* mBuf;

	CBuffer(int bufferSize=96000) {
		mBufferSize = bufferSize;
		mBuf = new float[mBufferSize];
		reset();
	}
    
    ~CBuffer() {
        // TODO: mBuf should be free'd?
    }

	void reset() {
		memset(mBuf, 0, mBufferSize*sizeof(float));
		mWriteIndex = 0;		
	}

    // depricated. use push instead
//    void write(float input) { push(input); }
    
	void push(float input) {
		mBuf[mWriteIndex] = input;

		mWriteIndex++;
		if(mWriteIndex >= mBufferSize)
			mWriteIndex = 0;
	}

    // index operator with 0 being the newest entry and
    // mBufferSize-1 being the oldest entry.
    // Note: it could be interesting to use negative indexes
    float index(int idx) {
        if(idx < 0)
            throw std::invalid_argument("received negative value");

        if(idx >= mBufferSize)
            idx = idx % mBufferSize;
        
        int diff = idx - mWriteIndex;

        if(diff < 0) {
            return mBuf[mWriteIndex - idx - 1];
        }
        return mBuf[mBufferSize - diff - 1];
    }
    
    float operator[](int idx) {
        return index(idx);
    }
    

    
    // currently, if samples is larger than mBufferSize, it gets modded down into range
    // future, make negative values work properly
	float read_num_samples(int samples) {
        if(samples < 0)
            throw std::invalid_argument("received negative value");
        
		if(samples > mBufferSize)
			samples = samples % mBufferSize;

		if(samples < mWriteIndex) {
			return mBuf[mWriteIndex-(samples+1)];
		}

		return mBuf[mBufferSize - (samples - mWriteIndex) - 1];
	}
    
    // purely for debugging
    void dump() {
        std::cout << "CBuffer Dump" << std::endl;
        std::cout << "\tmBufferSize: " << mBufferSize << std::endl <<
            "\tmWriteIndex: " << mWriteIndex << std::endl <<
            "\telements: [";
        for(int i=0; i<mBufferSize; i++)
            std::cout << mBuf[i] << ", ";
        std::cout << "]" << std::endl;
    }
};



