// (C) Copyright 2006-2009 Hewlett-Packard Development Company, L.P.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

// $Id$
#ifndef OPCODES_H
#define OPCODES_H

#include <hashmap.h>

class Opcodes
{
private:
	class Datum
	{
	private:
	    const uint8_t* opcode;
		uint32_t length;
		int type;
	public:
		Datum():opcode(0),length(0),type(0){}
		Datum(const uint8_t* o,uint32_t l,int t):opcode(o),length(l),type(t){}
	    inline const uint8_t* getOpcode() const { return opcode; }
	    inline uint32_t getLength() const { return length; }
	    inline int getType() const { return type; }
	};

	typedef HashMap<int64_t,Datum> Map;
	Map data;
	boost::pool<>* pul;

public:
    enum {pool_size=32, hash_size=200000}; // constants

	Opcodes();
	~Opcodes();

	// memory management
	void clear();
	inline uint8_t* malloc(int length)
	{
		int n=length/pool_size+1;
		if (n>1)
	        return static_cast<uint8_t*>(pul->ordered_malloc(n));
	    return static_cast<uint8_t*>(pul->malloc());
	}

	// accessing data
	typedef const Datum datum;
	inline const datum* find(uint64_t index) const 
	{
		Map::const_iterator i = data.find(index); 
	    return i == data.end() ? 0 : &(i->second);
	}

	inline void insert(uint64_t index,const uint8_t*opc,uint32_t length,int type)
	{
		data[index]=Datum(opc,length,type);
	}
};

#endif
