#pragma once

namespace Drn
{
	class Guid
	{
	public:
		Guid() : A(0), B(0), C(0), D(0) {}
		Guid(uint32 InA, uint32 InB, uint32 InC, uint32 InD)
			: A(InA), B(InB), C(InC), D(InD) {}

		static Guid NewGuid();

		friend bool operator==( const Guid& X, const Guid& Y )
		{
			return ((X.A ^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) == 0;
		}

		friend bool operator!=(const Guid& X, const Guid& Y)
		{
			return ((X.A ^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) != 0;
		}

		friend bool operator<(const Guid& X, const Guid& Y)
		{
			return	((X.A < Y.A) ? true : ((X.A > Y.A) ? false :
					((X.B < Y.B) ? true : ((X.B > Y.B) ? false :
					((X.C < Y.C) ? true : ((X.C > Y.C) ? false :
					((X.D < Y.D) ? true : ((X.D > Y.D) ? false : false))))))));
		}

		uint32& operator[](int32 Index)
		{
			if (!(Index >= 0 && Index < 4))
			{
				__debugbreak();
			}

			switch(Index)
			{
			case 0: return A;
			case 1: return B;
			case 2: return C;
			case 3: return D;
			}

			return A;
		}

		const uint32& operator[](int32 Index) const
		{
			if (!(Index >= 0 && Index < 4))
			{
				__debugbreak();
			}

			switch(Index)
			{
			case 0: return A;
			case 1: return B;
			case 2: return C;
			case 3: return D;
			}

			return A;
		}

		inline void Invalidate() { A = B = C = D = 0; }
		inline bool IsValid() const { return ((A | B | C | D) != 0); }

		std::string ToString() const;

		inline uint32 GetA() const { return A; }
		inline uint32 GetB() const { return B; }
		inline uint32 GetC() const { return C; }
		inline uint32 GetD() const { return D; }

	private:
		uint32 A;
		uint32 B;
		uint32 C;
		uint32 D;

		friend class Archive;
		friend class FileArchive;
		friend class BufferArchive;
	};
}