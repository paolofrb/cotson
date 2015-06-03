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
#ifndef READ_GZIP_H
#define READ_GZIP_H

#include <zlib.h>
#include <string>

class ReadGzip 
{
	public: 
	ReadGzip(std::string gzName): ok(true),data(true)
	{
		f=gzopen(gzName.c_str(),"rb");
		if(f==NULL)
			ok=false;
	}
	~ReadGzip() { if(ok) gzclose(f); }
	template<typename T>
	ReadGzip& operator>>(T&);

	operator const void*() const { return ok&&data ? this : 0; }
	bool eof() const { return !data || gzeof(f); }

	std::string as_text(size_t n)
	{
		if(ok)
		{
			char buf[n];
			size_t w=gzread(f,buf,n);
			if(w!=n) 
				ok=false;
		    return std::string(buf);
		}
		return std::string();
	}

	private:
	bool ok,data;
	gzFile f;
};

template<typename T>
ReadGzip& ReadGzip::operator>>(T& t)
{
	if(ok&&data)
	{
		int w=gzread(f,&t,sizeof(t));
		if ((w<=0) || gzeof(f))
			data=false;
		else if(w!=sizeof(t))
			ok=false;
	}
	return *this;
}
#endif
