//
//  File: DKHash.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include <memory.h>
#include <math.h>
#include "DKHash.h"
#include "DKEndianness.h"


////////////////////////////////////////////////////////////////////////////////
//
// Note:
//  MD5 was implemented based on RFC 1320, 1321
//  SHA1 was implemented based on NIST FIPS 18001, RFC 3174
//  SHA256,384,512 was implemented based on NIST FIPS 180-2
//
////////////////////////////////////////////////////////////////////////////////

#define HASH_LEFT_ROTATE32(x, c)				(((x) << (c)) | ((x) >> (32 - (c))))
#define HASH_RIGHT_ROTATE32(x, c)				(((x) >> (c)) | ((x) << (32 - (c))))

#define HASH_LEFT_ROTATE64(x, c)				(((x) << (c)) | ((x) >> (64 - (c))))
#define HASH_RIGHT_ROTATE64(x, c)				(((x) >> (c)) | ((x) << (64 - (c))))


////////////////////////////////////////////////////////////////////////////////
// Context
// store hash result, varies size by hash algorithm.
// hash32 (CRC32) uses hash[0] only.
// hash128 (MD5) uses hash[0]~[3].
// hash160 (SHA-1) uses hash[0]~[4].
// hash224,256 (SHA-224, SHA-256) uses hash[0]~[7].
// hash384,512 (SHA-384, SHA-512) uses hash[0]~[15].
struct DKFoundation::DKHash::Context
{
	union {
		uint64_t	hash64[8];
		uint32_t	hash32[16];
	};
	// saves message length (length x 8)
	uint64_t low; // md5, sha1, sha224/256 could save 64bits.(56bits actually)
	uint64_t high; // sha384/512 could save 128bits.(120bits actually)

	union {
		uint64_t	data64[16]; // use 16 for sha384/512, otherwise 8.
		uint32_t	data32[32];
		uint8_t		data8[128];
	};
	uint32_t num; // remains length (not processed)
	uint32_t len; // message hash length (bytes)
};

namespace DKFoundation
{
	namespace Private
	{
		using HashContext = DKHash::Context;
		////////////////////////////////////////////////////////////////////////////////
		// init context
		static inline void HashInit32(HashContext* ctx)
		{
			memset(ctx, 0, sizeof(HashContext));
			ctx->len = 4;
		}

		static inline void HashInit128(HashContext* ctx)
		{
			memset(ctx, 0, sizeof(HashContext));
			ctx->hash32[0] = (unsigned long)0x67452301;
			ctx->hash32[1] = (unsigned long)0xEFCDAB89;
			ctx->hash32[2] = (unsigned long)0x98BADCFE;
			ctx->hash32[3] = (unsigned long)0x10325476;
			ctx->len = 16;
		}

		static inline void HashInit160(HashContext* ctx)
		{
			memset(ctx, 0, sizeof(HashContext));
			ctx->hash32[0] = (unsigned long)0x67452301;
			ctx->hash32[1] = (unsigned long)0xefcdab89;
			ctx->hash32[2] = (unsigned long)0x98badcfe;
			ctx->hash32[3] = (unsigned long)0x10325476;
			ctx->hash32[4] = (unsigned long)0xc3d2e1f0;
			ctx->len = 20;
		}

		static inline void HashInit224(HashContext* ctx)
		{
			memset(ctx, 0, sizeof(HashContext));
			ctx->hash32[0] = (unsigned long)0xc1059ed8;
			ctx->hash32[1] = (unsigned long)0x367cd507;
			ctx->hash32[2] = (unsigned long)0x3070dd17;
			ctx->hash32[3] = (unsigned long)0xf70e5939;
			ctx->hash32[4] = (unsigned long)0xffc00b31;
			ctx->hash32[5] = (unsigned long)0x68581511;
			ctx->hash32[6] = (unsigned long)0x64f98fa7;
			ctx->hash32[7] = (unsigned long)0xbefa4fa4;
			ctx->len = 28;
		}

		static inline void HashInit256(HashContext* ctx)
		{
			memset(ctx, 0, sizeof(HashContext));
			ctx->hash32[0] = (unsigned long)0x6a09e667;
			ctx->hash32[1] = (unsigned long)0xbb67ae85;
			ctx->hash32[2] = (unsigned long)0x3c6ef372;
			ctx->hash32[3] = (unsigned long)0xa54ff53a;
			ctx->hash32[4] = (unsigned long)0x510e527f;
			ctx->hash32[5] = (unsigned long)0x9b05688c;
			ctx->hash32[6] = (unsigned long)0x1f83d9ab;
			ctx->hash32[7] = (unsigned long)0x5be0cd19;
			ctx->len = 32;
		}

		static inline void HashInit384(HashContext* ctx)
		{
			memset(ctx, 0, sizeof(HashContext));
			ctx->hash64[0] = 0xcbbb9d5dc1059ed8ULL;
			ctx->hash64[1] = 0x629a292a367cd507ULL;
			ctx->hash64[2] = 0x9159015a3070dd17ULL;
			ctx->hash64[3] = 0x152fecd8f70e5939ULL;
			ctx->hash64[4] = 0x67332667ffc00b31ULL;
			ctx->hash64[5] = 0x8eb44a8768581511ULL;
			ctx->hash64[6] = 0xdb0c2e0d64f98fa7ULL;
			ctx->hash64[7] = 0x47b5481dbefa4fa4ULL;
			ctx->len = 48;
		}

		static inline void HashInit512(HashContext* ctx)
		{
			memset(ctx, 0, sizeof(HashContext));
			ctx->hash64[0] = 0x6a09e667f3bcc908ULL;
			ctx->hash64[1] = 0xbb67ae8584caa73bULL;
			ctx->hash64[2] = 0x3c6ef372fe94f82bULL;
			ctx->hash64[3] = 0xa54ff53a5f1d36f1ULL;
			ctx->hash64[4] = 0x510e527fade682d1ULL;
			ctx->hash64[5] = 0x9b05688c2b3e6c1fULL;
			ctx->hash64[6] = 0x1f83d9abfb41bd6bULL;
			ctx->hash64[7] = 0x5be0cd19137e2179ULL;
			ctx->len = 64;
		}

		////////////////////////////////////////////////////////////////////////////////
		// update context digest
		static void HashUpdate32(HashContext* ctx, const void* p, size_t len)
		{
			static const unsigned int K[] = 
			{
				0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
				0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
				0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
				0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
				0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
				0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
				0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
				0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
				0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
				0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
				0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
				0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
				0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
				0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
				0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
				0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
				0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
				0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
				0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
				0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
				0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
				0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
				0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
				0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
				0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
				0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
				0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
				0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
				0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
				0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
				0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
				0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
			};

			unsigned int crc = ~(ctx->hash32[0]);

			for (unsigned int i = 0; i < len; i++)
				crc = K[ (crc ^ ((const char*)p)[i]) & 0xff] ^ (crc >> 8);

			ctx->hash32[0] = ~crc;
		}

		static void HashDigest128(HashContext* ctx, const void* p, size_t count)
		{
			unsigned int A,B,C,D,F;
			unsigned int W[16];

			static const unsigned int R[] ={
				7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,	// round 0
				5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,	// round 1
				4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,	// round 2
				6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21	// round 3
			};
			static const unsigned int K[] = {
				0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
				0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
				0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
				0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
				0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
				0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
				0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
				0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
			};

			for (unsigned int i = 0; i < count; i++)
			{
				A = ctx->hash32[0];
				B = ctx->hash32[1];
				C = ctx->hash32[2];
				D = ctx->hash32[3];

				for (int x = 0; x < 16; x++)
				{
					W[x] = DKSystemToLittleEndian( reinterpret_cast<const uint32_t*>(p)[ i * 16 + x ]);
				}

				int n = 0;
				unsigned int tmp;
				for (; n < 16; n++)
				{
					F = ((B & C) | ((~B) & D));
					tmp = D; D = C; C = B;
					B += HASH_LEFT_ROTATE32( A + F + K[n] + W[n] , R[n] );
					A = tmp;
				}
				for (; n < 32; n++)
				{
					F = ((B & D) | ((~D) & C));
					tmp = D; D = C; C = B;
					B += HASH_LEFT_ROTATE32( A + F + K[n] + W[(5*n+1) % 16] , R[n] );
					A = tmp;
				}
				for (; n < 48; n++)
				{
					F = (B ^ C ^ D);
					tmp = D; D = C; C = B;
					B += HASH_LEFT_ROTATE32( A + F + K[n] + W[(3*n+5) % 16] , R[n] );
					A = tmp;
				}
				for (; n < 64; n++)
				{
					F = (C ^ (B | (~D)));
					tmp = D; D = C; C = B;
					B += HASH_LEFT_ROTATE32( A + F + K[n] + W[(7*n) % 16] , R[n] );
					A = tmp;
				}

				ctx->hash32[0] += A;
				ctx->hash32[1] += B;
				ctx->hash32[2] += C;
				ctx->hash32[3] += D;
			}
		}

		static void HashDigest160(HashContext* ctx, const void* p, size_t count)
		{
			unsigned int A,B,C,D,E,T;
			unsigned int W[80];

			for (unsigned int i = 0; i < count; i++)
			{
				A = ctx->hash32[0];
				B = ctx->hash32[1];
				C = ctx->hash32[2];
				D = ctx->hash32[3];
				E = ctx->hash32[4];

				for (int x = 0; x < 16; x++)
				{
					W[x] = DKSystemToBigEndian(reinterpret_cast<const uint32_t*>(p)[ i * 16 + x ]);
				}
				for (int x = 16; x < 80; x++)
				{
					W[x] = HASH_LEFT_ROTATE32(W[x-3] ^ W[x-8] ^ W[x-14] ^ W[x-16], 1);
				}

				int n = 0;
				for (; n < 20; n++)
				{
					T = HASH_LEFT_ROTATE32(A, 5) + ((B & C) | ((~B) & D)) + E + W[n] + 0x5A827999;
					E = D; D = C; C = HASH_LEFT_ROTATE32(B, 30); B = A; A = T;
				}
				for (; n < 40; n++)
				{
					T = HASH_LEFT_ROTATE32(A, 5) + (B ^ C ^ D) + E + W[n] + 0x6ED9EBA1;
					E = D; D = C; C = HASH_LEFT_ROTATE32(B, 30); B = A; A = T;
				}
				for (; n < 60; n++)
				{
					T = HASH_LEFT_ROTATE32(A, 5) + ((B & C) | (B & D) | (C & D)) + E + W[n] + 0x8F1BBCDC;
					E = D; D = C; C = HASH_LEFT_ROTATE32(B, 30); B = A; A = T;
				}
				for (; n < 80; n++)
				{
					T = HASH_LEFT_ROTATE32(A, 5) + (B ^ C ^ D) + E + W[n] + 0xCA62C1D6;
					E = D; D = C; C = HASH_LEFT_ROTATE32(B, 30); B = A; A = T;
				}
				ctx->hash32[0] += A;
				ctx->hash32[1] += B;
				ctx->hash32[2] += C;
				ctx->hash32[3] += D;
				ctx->hash32[4] += E;
			}
		}

		static void HashDigest256(HashContext* ctx, const void* p, size_t count)
		{
			unsigned int A,B,C,D,E,F,G,H;
			static const unsigned int K[] = {
				0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
				0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
				0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
				0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
				0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
				0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
				0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
				0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
			};
			unsigned int W[64];
			for (unsigned int i = 0; i < count; i++)
			{
				A = ctx->hash32[0];
				B = ctx->hash32[1];
				C = ctx->hash32[2];
				D = ctx->hash32[3];
				E = ctx->hash32[4];
				F = ctx->hash32[5];
				G = ctx->hash32[6];
				H = ctx->hash32[7];

				for (int x = 0; x < 16; x++)
				{
					W[x] = DKSystemToBigEndian(reinterpret_cast<const uint32_t*>(p)[ i * 16 + x ]);
				}
				for (int x = 16; x < 64; x++)
				{
					unsigned int s0 = HASH_RIGHT_ROTATE32(W[x-15],7) ^ HASH_RIGHT_ROTATE32(W[x-15],18) ^ (W[x-15] >> 3);
					unsigned int s1 = HASH_RIGHT_ROTATE32(W[x-2],17) ^ HASH_RIGHT_ROTATE32(W[x-2],19) ^ (W[x-2] >> 10);
					W[x] = W[x-16] + s0 + W[x-7] + s1;
				}

				unsigned int s0, s1;
				unsigned int maj;
				unsigned int t1, t2;
				unsigned int ch;
				for (int n = 0; n < 64; n++)
				{
					s0 = HASH_RIGHT_ROTATE32(A,2) ^ HASH_RIGHT_ROTATE32(A,13) ^ HASH_RIGHT_ROTATE32(A,22);
					maj = (A & B) ^ (A & C) ^ (B & C);
					t2 = s0 + maj;
					s1 = HASH_RIGHT_ROTATE32(E,6) ^ HASH_RIGHT_ROTATE32(E,11) ^ HASH_RIGHT_ROTATE32(E,25);
					ch = (E & F) ^ ((~E) & G);
					t1 = H + s1 + ch + K[n] + W[n];

					H = G;
					G = F;
					F = E;
					E = D + t1;
					D = C;
					C = B;
					B = A;
					A = t1 + t2;
				}

				ctx->hash32[0] += A;
				ctx->hash32[1] += B;
				ctx->hash32[2] += C;
				ctx->hash32[3] += D;
				ctx->hash32[4] += E;
				ctx->hash32[5] += F;
				ctx->hash32[6] += G;
				ctx->hash32[7] += H;
			}
		}

		static void HashDigest512(HashContext* ctx, const void* p, size_t count)
		{
			unsigned long long A,B,C,D,E,F,G,H;
			static const unsigned long long K[] = {
				0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL, 
				0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL, 
				0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL, 
				0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL, 
				0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL, 
				0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL, 
				0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL, 
				0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL, 
				0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL, 
				0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL, 
				0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL, 
				0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL, 
				0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL, 
				0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL, 
				0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL, 
				0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL, 
				0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL, 
				0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL, 
				0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL, 
				0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
			};
			unsigned long long W[80];
			for (unsigned int i = 0; i < count; i++)
			{
				A = ctx->hash64[0];
				B = ctx->hash64[1];
				C = ctx->hash64[2];
				D = ctx->hash64[3];
				E = ctx->hash64[4];
				F = ctx->hash64[5];
				G = ctx->hash64[6];
				H = ctx->hash64[7];

				for (int x = 0; x < 16; x++)
				{
					W[x] = DKSystemToBigEndian(reinterpret_cast<const uint64_t*>(p)[ i * 16 + x ]);
				}
				for (int x = 16; x < 80; x++)
				{
					unsigned long long s0 = HASH_RIGHT_ROTATE64(W[x-15],1) ^ HASH_RIGHT_ROTATE64(W[x-15],8) ^ (W[x-15] >> 7);
					unsigned long long s1 = HASH_RIGHT_ROTATE64(W[x-2],19) ^ HASH_RIGHT_ROTATE64(W[x-2],61) ^ (W[x-2] >> 6);
					W[x] = W[x-16] + s0 + W[x-7] + s1;
				}

				unsigned long long s0, s1;
				unsigned long long maj;
				unsigned long long t1, t2;
				unsigned long long ch;
				for (int n = 0; n < 80; n++)
				{
					s0 = HASH_RIGHT_ROTATE64(A,28) ^ HASH_RIGHT_ROTATE64(A,34) ^ HASH_RIGHT_ROTATE64(A,39);
					maj = (A & B) ^ (A & C) ^ (B & C);
					t2 = s0 + maj;
					s1 = HASH_RIGHT_ROTATE64(E,14) ^ HASH_RIGHT_ROTATE64(E,18) ^ HASH_RIGHT_ROTATE64(E,41);
					ch = (E & F) ^ ((~E) & G);
					t1 = H + s1 + ch + K[n] + W[n];

					H = G;
					G = F;
					F = E;
					E = D + t1;
					D = C;
					C = B;
					B = A;
					A = t1 + t2;
				}

				ctx->hash64[0] += A;
				ctx->hash64[1] += B;
				ctx->hash64[2] += C;
				ctx->hash64[3] += D;
				ctx->hash64[4] += E;
				ctx->hash64[5] += F;
				ctx->hash64[6] += G;
				ctx->hash64[7] += H;
			}
		}

		// HashUpdate : updates hash digest, using all algorithms except for CRC32
		template <typename HashDigest> static void HashUpdate(HashContext* ctx, size_t block_size, const void* p, size_t len, HashDigest hash_func)
		{
			const unsigned char *data = (const unsigned char*)p;

			if (len == 0)
				return;

			// 4 bytes integer type if cBlock is 64
			// 8 bytes integer type if cBlock is 128
			size_t baseTypeSize = block_size / 16;

			if (baseTypeSize > 4)
			{
				unsigned long long len2 = ctx->low + (((unsigned long long)len) << 3);
				if (len2 < ctx->low) // overflow!
					ctx->high++;
				ctx->high += ((unsigned long long)len) >> 61;
				ctx->low = len2;
			}
			else
			{
				ctx->low += ((unsigned long long)len) << 3;
			}

			size_t n = ctx->num;
			if (n != 0)
			{
				if (len >= block_size || len+n >= block_size)
				{
					memcpy(ctx->data8 + n, data, block_size - n);
					hash_func(ctx, ctx->data8, 1);
					n = (size_t)(block_size - n);
					data += n;
					len -= n;
					ctx->num = 0;
					memset(ctx->data8, 0, (size_t)block_size);
				}
				else
				{
					memcpy(ctx->data8 + n, data, len);
					ctx->num += (unsigned int)len;
					return;
				}
			}
			n = len / block_size;
			if (n > 0)
			{
				hash_func(ctx, data, n);
				n *= block_size;
				data += n;
				len -= n;
			}
			if (len != 0)
			{
				ctx->num = (unsigned int)len;
				memcpy(ctx->data8, data, len);
			}
		}

		template <typename HashDigest> static void HashFinal(HashContext* ctx, size_t block_size, HashDigest hash_func, bool bLittleEndian)
		{
			size_t n = ctx->num;

			// 1. add one bit to end of messages. (MD5, SHA1)
			// 2. fill message's 512bits with 0. (64bytes, required length for one hash)
			// 3. add total message length as big-endian integer.
			ctx->data8[n++] = 0x80;

			// 4 bytes integer type if cBlock is 64.
			// 8 bytes integer type if cBlock is 128.
			size_t baseTypeSize = block_size / 16;

			if (n > block_size - baseTypeSize*2)
			{
				memset(ctx->data8 + n, 0, (size_t)(block_size - n));
				n = 0;
				hash_func(ctx, ctx->data8, 1);
			}
			// fill with 0, except for last 2 blocks
			memset(ctx->data8 + n, 0, (size_t)(block_size - n - (baseTypeSize * 2)));

			// set last 2 blocks with low, high values.
			if (baseTypeSize > 4)
			{
				if (bLittleEndian)
				{
					ctx->data64[14] = DKSystemToLittleEndian(ctx->low);
					ctx->data64[15] = DKSystemToLittleEndian(ctx->high);
				}
				else
				{
					ctx->data64[14] = DKSystemToBigEndian(ctx->high);
					ctx->data64[15] = DKSystemToBigEndian(ctx->low);
				}
			}
			else
			{
				if (bLittleEndian)
				{
					ctx->data64[7] = DKSystemToLittleEndian(ctx->low);
				}
				else
				{
					ctx->data64[7] = DKSystemToBigEndian(ctx->low);
				}
			}

			hash_func(ctx, ctx->data8, 1);
			ctx->num = 0;

#ifdef __LITTLE_ENDIAN__
			// SHA Family calculated as Big-Endian, swapping bytes-order.
			if (baseTypeSize > 4 && !bLittleEndian)
			{
				for (int i = 0; i < 8; i++)
				{
					unsigned int tmp = ctx->hash32[i*2];
					ctx->hash32[i*2] = ctx->hash32[i*2+1];
					ctx->hash32[i*2+1] = tmp;
				}
			}
#endif
		}
	}
}


namespace DKFoundation
{
#define DEBUG_CHECK_RUNTIME_ENDIANNESS	DKASSERT_DESC_DEBUG(DKVerifyByteOrder(), "System Byte-Order Mismatch!")

	DKHashResult32 DKGL_API DKHashCRC32(const void* p, size_t len)
	{
		DEBUG_CHECK_RUNTIME_ENDIANNESS;
		Private::HashContext ctx;
		Private::HashInit32(&ctx);
		Private::HashUpdate32(&ctx, p, len);

		DKHashResult32	res;
		res.digest[0] = ctx.hash32[0];
		return res;
	}
	DKHashResult128 DKGL_API DKHashMD5(const void* p, size_t len)
	{
		DEBUG_CHECK_RUNTIME_ENDIANNESS;
		Private::HashContext ctx;
		Private::HashInit128(&ctx);
		Private::HashUpdate(&ctx, 64, p, len, Private::HashDigest128);
		Private::HashFinal(&ctx, 64, Private::HashDigest128, true);

		DKHashResult128	res;
		for (int i = 0; i < 4; i++)
			res.digest[i] = DKSwitchIntegralByteOrder(ctx.hash32[i]);
		return res;
	}
	DKHashResult160 DKGL_API DKHashSHA1(const void* p, size_t len)
	{
		DEBUG_CHECK_RUNTIME_ENDIANNESS;
		Private::HashContext ctx;
		Private::HashInit160(&ctx);
		Private::HashUpdate(&ctx, 64, p, len, Private::HashDigest160);
		Private::HashFinal(&ctx, 64, Private::HashDigest160, false);

		DKHashResult160 res;
		for (int i = 0; i < 5; i++)
			res.digest[i] = ctx.hash32[i];
		return res;
	}
	DKHashResult224 DKGL_API DKHashSHA224(const void* p, size_t len)
	{
		DEBUG_CHECK_RUNTIME_ENDIANNESS;
		Private::HashContext ctx;
		Private::HashInit224(&ctx);
		Private::HashUpdate(&ctx, 64, p, len, Private::HashDigest256);
		Private::HashFinal(&ctx, 64, Private::HashDigest256, false);

		DKHashResult224 res;
		for (int i = 0; i < 7; i++)
			res.digest[i] = ctx.hash32[i];
		return res;
	}
	DKHashResult256 DKGL_API DKHashSHA256(const void* p, size_t len)
	{
		DEBUG_CHECK_RUNTIME_ENDIANNESS;
		Private::HashContext ctx;
		Private::HashInit256(&ctx);
		Private::HashUpdate(&ctx, 64, p, len, Private::HashDigest256);
		Private::HashFinal(&ctx, 64, Private::HashDigest256, false);

		DKHashResult256 res;
		for (int i = 0; i < 8; i++)
			res.digest[i] = ctx.hash32[i];
		return res;
	}
	DKHashResult384 DKGL_API DKHashSHA384(const void* p, size_t len)
	{
		DEBUG_CHECK_RUNTIME_ENDIANNESS;
		Private::HashContext ctx;
		Private::HashInit384(&ctx);
		Private::HashUpdate(&ctx, 128, p, len, Private::HashDigest512);
		Private::HashFinal(&ctx, 128, Private::HashDigest512, false);

		DKHashResult384 res;
		for (int i = 0; i < 12; i++)
			res.digest[i] = ctx.hash32[i];
		return res;
	}
	DKHashResult512 DKGL_API DKHashSHA512(const void* p, size_t len)
	{
		DEBUG_CHECK_RUNTIME_ENDIANNESS;
		Private::HashContext ctx;
		Private::HashInit512(&ctx);
		Private::HashUpdate(&ctx, 128, p, len, Private::HashDigest512);
		Private::HashFinal(&ctx, 128, Private::HashDigest512, false);

		DKHashResult512 res;
		for (int i = 0; i < 16; i++)
			res.digest[i] = ctx.hash32[i];
		return res;
	}
}

using namespace DKFoundation;

DKHash::DKHash(Type t)
	: type(t)
	, finalized(true)
	, ctxt(NULL)
{
	DEBUG_CHECK_RUNTIME_ENDIANNESS;
}

DKHash::~DKHash(void)
{
	if (ctxt)
		delete ctxt;
}

void DKHash::Initialize(void)
{
	if (ctxt == NULL)
		ctxt = new Private::HashContext();

	switch (type)
	{
	case Type32:
		Private::HashInit32(ctxt);
		break;
	case Type128:
		Private::HashInit128(ctxt);
		break;
	case Type160:
		Private::HashInit160(ctxt);
		break;
	case Type224:
		Private::HashInit224(ctxt);
		break;
	case Type256:
		Private::HashInit256(ctxt);
		break;
	case Type384:
		Private::HashInit384(ctxt);
		break;
	case Type512:
		Private::HashInit512(ctxt);
		break;
	default:
		DKERROR_THROW_DEBUG("Uknown type");
		break;
	}
	finalized = false;
}

void DKHash::Update(const void* p, size_t len)
{
	DKASSERT_DESC_DEBUG(finalized == false, "Hash object not initialized.");

	switch (type)
	{
	case Type32:
		Private::HashUpdate32(ctxt, p, len);
		break;
	case Type128:
		Private::HashUpdate(ctxt, 64, p, len, Private::HashDigest128);
		break;
	case Type160:
		Private::HashUpdate(ctxt, 64, p, len, Private::HashDigest160);
		break;
	case Type224:
		Private::HashUpdate(ctxt, 64, p, len, Private::HashDigest256);
		break;
	case Type256:
		Private::HashUpdate(ctxt, 64, p, len, Private::HashDigest256);
		break;
	case Type384:
		Private::HashUpdate(ctxt, 128, p, len, Private::HashDigest512);
		break;
	case Type512:
		Private::HashUpdate(ctxt, 128, p, len, Private::HashDigest512);
		break;
	default:
		DKERROR_THROW_DEBUG("Uknown type");
		break;
	}
}

void DKHash::Finalize(void)
{
	if (!finalized)
	{
		switch (type)
		{
		case Type32:
			break;
		case Type128:
			Private::HashFinal(ctxt, 64, Private::HashDigest128, true);
			break;
		case Type160:
			Private::HashFinal(ctxt, 64, Private::HashDigest160, false);
			break;
		case Type224:
			Private::HashFinal(ctxt, 64, Private::HashDigest256, false);
			break;
		case Type256:
			Private::HashFinal(ctxt, 64, Private::HashDigest256, false);
			break;
		case Type384:
			Private::HashFinal(ctxt, 128, Private::HashDigest512, false);
			break;
		case Type512:
			Private::HashFinal(ctxt, 128, Private::HashDigest512, false);
			break;
		default:
			DKERROR_THROW_DEBUG("Uknown type");
			break;
		}
		finalized = true;
	}
}

DKHashResult32 DKHash32::Result(void) const
{
	DKASSERT_DESC_DEBUG(type == Type32, "Invalid Hash Type");
	DKASSERT_DESC_DEBUG(ctxt != NULL, "Object not initialized");

	DKHashResult32	res;
	res.digest[0] = ctxt->hash32[0];
	return res;
}

DKHashResult128 DKHash128::Result(void) const
{
	DKASSERT_DESC_DEBUG(type == Type128, "Invalid Hash Type");
	DKASSERT_DESC_DEBUG(ctxt != NULL, "Object not initialized.");
	DKASSERT_DESC_DEBUG(finalized, "Object not finalized.");

	DKHashResult128	res;
	for (int i = 0; i < 4; i++)
		res.digest[i] = DKSwitchIntegralByteOrder(ctxt->hash32[i]);
	return res;
}

DKHashResult160 DKHash160::Result(void) const
{
	DKASSERT_DESC_DEBUG(type == Type160, "Invalid Hash Type");
	DKASSERT_DESC_DEBUG(ctxt != NULL, "Object not initialized.");
	DKASSERT_DESC_DEBUG(finalized, "Object not finalized.");

	DKHashResult160 res;
	for (int i = 0; i < 5; i++)
		res.digest[i] = ctxt->hash32[i];
	return res;
}

DKHashResult224 DKHash224::Result(void) const
{
	DKASSERT_DESC_DEBUG(type == Type224, "Invalid Hash Type");
	DKASSERT_DESC_DEBUG(ctxt != NULL, "Object not initialized.");
	DKASSERT_DESC_DEBUG(finalized, "Object not finalized.");

	DKHashResult224 res;
	for (int i = 0; i < 7; i++)
		res.digest[i] = ctxt->hash32[i];
	return res;
}

DKHashResult256 DKHash256::Result(void) const
{
	DKASSERT_DESC_DEBUG(type == Type256, "Invalid Hash Type");
	DKASSERT_DESC_DEBUG(ctxt != NULL, "Object not initialized.");
	DKASSERT_DESC_DEBUG(finalized, "Object not finalized.");

	DKHashResult256 res;
	for (int i = 0; i < 8; i++)
		res.digest[i] = ctxt->hash32[i];
	return res;
}

DKHashResult384 DKHash384::Result(void) const
{
	DKASSERT_DESC_DEBUG(type == Type384, "Invalid Hash Type");
	DKASSERT_DESC_DEBUG(ctxt != NULL, "Object not initialized.");
	DKASSERT_DESC_DEBUG(finalized, "Object not finalized.");

	DKHashResult384 res;
	for (int i = 0; i < 12; i++)
		res.digest[i] = ctxt->hash32[i];
	return res;
}

DKHashResult512 DKHash512::Result(void) const
{
	DKASSERT_DESC_DEBUG(type == Type512, "Invalid Hash Type");
	DKASSERT_DESC_DEBUG(ctxt != NULL, "Object not initialized.");
	DKASSERT_DESC_DEBUG(finalized, "Object not finalized.");

	DKHashResult512 res;
	for (int i = 0; i < 16; i++)
		res.digest[i] = ctxt->hash32[i];
	return res;
}
