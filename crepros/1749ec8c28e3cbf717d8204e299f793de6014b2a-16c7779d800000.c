// https://syzkaller.appspot.com/bug?id=1749ec8c28e3cbf717d8204e299f793de6014b2a
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfa9000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 6, 0);
  memcpy((void*)0x20fa2000, "\x6e\x61\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20fa2020 = 0x1b;
  *(uint32_t*)0x20fa2024 = 5;
  *(uint32_t*)0x20fa2028 = 0x4d8;
  *(uint32_t*)0x20fa202c = 0;
  *(uint32_t*)0x20fa2030 = 0x318;
  *(uint32_t*)0x20fa2034 = -1;
  *(uint32_t*)0x20fa2038 = 0x228;
  *(uint32_t*)0x20fa203c = 0x318;
  *(uint32_t*)0x20fa2040 = 0x408;
  *(uint32_t*)0x20fa2044 = 0x408;
  *(uint32_t*)0x20fa2048 = -1;
  *(uint32_t*)0x20fa204c = 0x408;
  *(uint32_t*)0x20fa2050 = 0x408;
  *(uint32_t*)0x20fa2054 = 5;
  *(uint64_t*)0x20fa2058 = 0x201affb0;
  *(uint8_t*)0x20fa2060 = -1;
  *(uint8_t*)0x20fa2061 = 1;
  *(uint8_t*)0x20fa2062 = 0;
  *(uint8_t*)0x20fa2063 = 0;
  *(uint8_t*)0x20fa2064 = 0;
  *(uint8_t*)0x20fa2065 = 0;
  *(uint8_t*)0x20fa2066 = 0;
  *(uint8_t*)0x20fa2067 = 0;
  *(uint8_t*)0x20fa2068 = 0;
  *(uint8_t*)0x20fa2069 = 0;
  *(uint8_t*)0x20fa206a = 0;
  *(uint8_t*)0x20fa206b = 0;
  *(uint8_t*)0x20fa206c = 0;
  *(uint8_t*)0x20fa206d = 0;
  *(uint8_t*)0x20fa206e = 0;
  *(uint8_t*)0x20fa206f = 1;
  *(uint8_t*)0x20fa2070 = 0;
  *(uint8_t*)0x20fa2071 = 0;
  *(uint8_t*)0x20fa2072 = 0;
  *(uint8_t*)0x20fa2073 = 0;
  *(uint8_t*)0x20fa2074 = 0;
  *(uint8_t*)0x20fa2075 = 0;
  *(uint8_t*)0x20fa2076 = 0;
  *(uint8_t*)0x20fa2077 = 0;
  *(uint8_t*)0x20fa2078 = 0;
  *(uint8_t*)0x20fa2079 = 0;
  *(uint8_t*)0x20fa207a = -1;
  *(uint8_t*)0x20fa207b = -1;
  *(uint32_t*)0x20fa207c = htobe32(-1);
  *(uint32_t*)0x20fa2080 = htobe32(0);
  *(uint32_t*)0x20fa2084 = htobe32(0);
  *(uint32_t*)0x20fa2088 = htobe32(0);
  *(uint32_t*)0x20fa208c = htobe32(0);
  *(uint32_t*)0x20fa2090 = htobe32(-1);
  *(uint32_t*)0x20fa2094 = htobe32(0);
  *(uint32_t*)0x20fa2098 = htobe32(0);
  *(uint32_t*)0x20fa209c = htobe32(0);
  memcpy((void*)0x20fa20a0,
         "\x69\x70\x36\x67\x72\x65\x74\x61\x70\x30\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x20fa20b0 = 0x73;
  *(uint8_t*)0x20fa20b1 = 0x79;
  *(uint8_t*)0x20fa20b2 = 0x7a;
  *(uint8_t*)0x20fa20b3 = 0;
  *(uint8_t*)0x20fa20b4 = 0;
  *(uint8_t*)0x20fa20c0 = 0;
  *(uint8_t*)0x20fa20c1 = 0;
  *(uint8_t*)0x20fa20c2 = 0;
  *(uint8_t*)0x20fa20c3 = 0;
  *(uint8_t*)0x20fa20c4 = 0;
  *(uint8_t*)0x20fa20c5 = 0;
  *(uint8_t*)0x20fa20c6 = 0;
  *(uint8_t*)0x20fa20c7 = 0;
  *(uint8_t*)0x20fa20c8 = 0;
  *(uint8_t*)0x20fa20c9 = 0;
  *(uint8_t*)0x20fa20ca = 0;
  *(uint8_t*)0x20fa20cb = 0;
  *(uint8_t*)0x20fa20cc = 0;
  *(uint8_t*)0x20fa20cd = 0;
  *(uint8_t*)0x20fa20ce = 0;
  *(uint8_t*)0x20fa20cf = 0;
  *(uint8_t*)0x20fa20d0 = 0;
  *(uint8_t*)0x20fa20d1 = 0;
  *(uint8_t*)0x20fa20d2 = 0;
  *(uint8_t*)0x20fa20d3 = 0;
  *(uint8_t*)0x20fa20d4 = 0;
  *(uint8_t*)0x20fa20d5 = 0;
  *(uint8_t*)0x20fa20d6 = 0;
  *(uint8_t*)0x20fa20d7 = 0;
  *(uint8_t*)0x20fa20d8 = 0;
  *(uint8_t*)0x20fa20d9 = 0;
  *(uint8_t*)0x20fa20da = 0;
  *(uint8_t*)0x20fa20db = 0;
  *(uint8_t*)0x20fa20dc = 0;
  *(uint8_t*)0x20fa20dd = 0;
  *(uint8_t*)0x20fa20de = 0;
  *(uint8_t*)0x20fa20df = 0;
  *(uint16_t*)0x20fa20e0 = 0;
  *(uint8_t*)0x20fa20e2 = 0;
  *(uint8_t*)0x20fa20e3 = 0;
  *(uint8_t*)0x20fa20e4 = 0;
  *(uint32_t*)0x20fa20e8 = 0;
  *(uint16_t*)0x20fa20ec = 0xa8;
  *(uint16_t*)0x20fa20ee = 0xf0;
  *(uint32_t*)0x20fa20f0 = 0;
  *(uint64_t*)0x20fa20f8 = 0;
  *(uint64_t*)0x20fa2100 = 0;
  *(uint16_t*)0x20fa2108 = 0x48;
  memcpy((void*)0x20fa210a, "\x4e\x45\x54\x4d\x41\x50\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa2127 = 0;
  *(uint32_t*)0x20fa2128 = 1;
  *(uint8_t*)0x20fa212c = 0;
  *(uint8_t*)0x20fa212d = 0;
  *(uint8_t*)0x20fa212e = 0;
  *(uint8_t*)0x20fa212f = 0;
  *(uint8_t*)0x20fa2130 = 0;
  *(uint8_t*)0x20fa2131 = 0;
  *(uint8_t*)0x20fa2132 = 0;
  *(uint8_t*)0x20fa2133 = 0;
  *(uint8_t*)0x20fa2134 = 0;
  *(uint8_t*)0x20fa2135 = 0;
  *(uint8_t*)0x20fa2136 = -1;
  *(uint8_t*)0x20fa2137 = -1;
  *(uint32_t*)0x20fa2138 = htobe32(0xe0000001);
  *(uint8_t*)0x20fa213c = 0xfe;
  *(uint8_t*)0x20fa213d = 0x80;
  *(uint8_t*)0x20fa213e = 0;
  *(uint8_t*)0x20fa213f = 0;
  *(uint8_t*)0x20fa2140 = 0;
  *(uint8_t*)0x20fa2141 = 0;
  *(uint8_t*)0x20fa2142 = 0;
  *(uint8_t*)0x20fa2143 = 0;
  *(uint8_t*)0x20fa2144 = 0;
  *(uint8_t*)0x20fa2145 = 0;
  *(uint8_t*)0x20fa2146 = 0;
  *(uint8_t*)0x20fa2147 = 0;
  *(uint8_t*)0x20fa2148 = 0;
  *(uint8_t*)0x20fa2149 = 0;
  *(uint8_t*)0x20fa214a = 0;
  *(uint8_t*)0x20fa214b = 0xbb;
  *(uint16_t*)0x20fa214c = 0;
  *(uint16_t*)0x20fa214e = 0;
  *(uint8_t*)0x20fa2150 = 0;
  *(uint8_t*)0x20fa2151 = 0;
  *(uint8_t*)0x20fa2152 = 0;
  *(uint8_t*)0x20fa2153 = 0;
  *(uint8_t*)0x20fa2154 = 0;
  *(uint8_t*)0x20fa2155 = 0;
  *(uint8_t*)0x20fa2156 = 0;
  *(uint8_t*)0x20fa2157 = 0;
  *(uint8_t*)0x20fa2158 = 0;
  *(uint8_t*)0x20fa2159 = 0;
  *(uint8_t*)0x20fa215a = 0;
  *(uint8_t*)0x20fa215b = 0;
  *(uint8_t*)0x20fa215c = 0;
  *(uint8_t*)0x20fa215d = 0;
  *(uint8_t*)0x20fa215e = 0;
  *(uint8_t*)0x20fa215f = 0;
  *(uint8_t*)0x20fa2160 = 0;
  *(uint8_t*)0x20fa2161 = 0;
  *(uint8_t*)0x20fa2162 = 0;
  *(uint8_t*)0x20fa2163 = 0;
  *(uint8_t*)0x20fa2164 = 0;
  *(uint8_t*)0x20fa2165 = 0;
  *(uint8_t*)0x20fa2166 = 0;
  *(uint8_t*)0x20fa2167 = 0;
  *(uint8_t*)0x20fa2168 = 0;
  *(uint8_t*)0x20fa2169 = 0;
  *(uint8_t*)0x20fa216a = 0;
  *(uint8_t*)0x20fa216b = 0;
  *(uint8_t*)0x20fa216c = 0;
  *(uint8_t*)0x20fa216d = 0;
  *(uint8_t*)0x20fa216e = 0;
  *(uint8_t*)0x20fa216f = 0;
  *(uint8_t*)0x20fa2170 = 0;
  *(uint8_t*)0x20fa2171 = 0;
  *(uint8_t*)0x20fa2172 = 0;
  *(uint8_t*)0x20fa2173 = 0;
  *(uint8_t*)0x20fa2174 = 0;
  *(uint8_t*)0x20fa2175 = 0;
  *(uint8_t*)0x20fa2176 = 0;
  *(uint8_t*)0x20fa2177 = 0;
  *(uint8_t*)0x20fa2178 = 0;
  *(uint8_t*)0x20fa2179 = 0;
  *(uint8_t*)0x20fa217a = 0;
  *(uint8_t*)0x20fa217b = 0;
  *(uint8_t*)0x20fa217c = 0;
  *(uint8_t*)0x20fa217d = 0;
  *(uint8_t*)0x20fa217e = 0;
  *(uint8_t*)0x20fa217f = 0;
  *(uint8_t*)0x20fa2180 = 0;
  *(uint8_t*)0x20fa2181 = 0;
  *(uint8_t*)0x20fa2182 = 0;
  *(uint8_t*)0x20fa2183 = 0;
  *(uint8_t*)0x20fa2184 = 0;
  *(uint8_t*)0x20fa2185 = 0;
  *(uint8_t*)0x20fa2186 = 0;
  *(uint8_t*)0x20fa2187 = 0;
  *(uint8_t*)0x20fa2188 = 0;
  *(uint8_t*)0x20fa2189 = 0;
  *(uint8_t*)0x20fa218a = 0;
  *(uint8_t*)0x20fa218b = 0;
  *(uint8_t*)0x20fa218c = 0;
  *(uint8_t*)0x20fa218d = 0;
  *(uint8_t*)0x20fa218e = 0;
  *(uint8_t*)0x20fa218f = 0;
  *(uint8_t*)0x20fa2190 = 0;
  *(uint8_t*)0x20fa2191 = 0;
  *(uint8_t*)0x20fa2192 = 0;
  *(uint8_t*)0x20fa2193 = 0;
  *(uint8_t*)0x20fa2194 = 0;
  *(uint8_t*)0x20fa2195 = 0;
  *(uint8_t*)0x20fa2196 = 0;
  *(uint8_t*)0x20fa2197 = 0;
  *(uint8_t*)0x20fa2198 = 0;
  *(uint8_t*)0x20fa2199 = 0;
  *(uint8_t*)0x20fa219a = 0;
  *(uint8_t*)0x20fa219b = 0;
  *(uint8_t*)0x20fa219c = 0;
  *(uint8_t*)0x20fa219d = 0;
  *(uint8_t*)0x20fa219e = 0;
  *(uint8_t*)0x20fa219f = 0;
  *(uint8_t*)0x20fa21a0 = 0;
  *(uint8_t*)0x20fa21a1 = 0;
  *(uint8_t*)0x20fa21a2 = 0;
  *(uint8_t*)0x20fa21a3 = 0;
  *(uint8_t*)0x20fa21a4 = 0;
  *(uint8_t*)0x20fa21a5 = 0;
  *(uint8_t*)0x20fa21a6 = 0;
  *(uint8_t*)0x20fa21a7 = 0;
  *(uint8_t*)0x20fa21a8 = 0;
  *(uint8_t*)0x20fa21a9 = 0;
  *(uint8_t*)0x20fa21aa = 0;
  *(uint8_t*)0x20fa21ab = 0;
  *(uint8_t*)0x20fa21ac = 0;
  *(uint8_t*)0x20fa21ad = 0;
  *(uint8_t*)0x20fa21ae = 0;
  *(uint8_t*)0x20fa21af = 0;
  *(uint8_t*)0x20fa21b0 = 0;
  *(uint8_t*)0x20fa21b1 = 0;
  *(uint8_t*)0x20fa21b2 = 0;
  *(uint8_t*)0x20fa21b3 = 0;
  *(uint8_t*)0x20fa21b4 = 0;
  *(uint8_t*)0x20fa21b5 = 0;
  *(uint8_t*)0x20fa21b6 = 0;
  *(uint8_t*)0x20fa21b7 = 0;
  *(uint8_t*)0x20fa21b8 = 0;
  *(uint8_t*)0x20fa21b9 = 0;
  *(uint8_t*)0x20fa21ba = 0;
  *(uint8_t*)0x20fa21bb = 0;
  *(uint8_t*)0x20fa21bc = 0;
  *(uint8_t*)0x20fa21bd = 0;
  *(uint8_t*)0x20fa21be = 0;
  *(uint8_t*)0x20fa21bf = 0;
  *(uint8_t*)0x20fa21c0 = 0;
  *(uint8_t*)0x20fa21c1 = 0;
  *(uint8_t*)0x20fa21c2 = 0;
  *(uint8_t*)0x20fa21c3 = 0;
  *(uint8_t*)0x20fa21c4 = 0;
  *(uint8_t*)0x20fa21c5 = 0;
  *(uint8_t*)0x20fa21c6 = 0;
  *(uint8_t*)0x20fa21c7 = 0;
  *(uint8_t*)0x20fa21c8 = 0;
  *(uint8_t*)0x20fa21c9 = 0;
  *(uint8_t*)0x20fa21ca = 0;
  *(uint8_t*)0x20fa21cb = 0;
  *(uint8_t*)0x20fa21cc = 0;
  *(uint8_t*)0x20fa21cd = 0;
  *(uint8_t*)0x20fa21ce = 0;
  *(uint8_t*)0x20fa21cf = 0;
  *(uint8_t*)0x20fa21d0 = 0;
  *(uint8_t*)0x20fa21d1 = 0;
  *(uint8_t*)0x20fa21d2 = 0;
  *(uint8_t*)0x20fa21d3 = 0;
  *(uint8_t*)0x20fa21d4 = 0;
  *(uint8_t*)0x20fa21d5 = 0;
  *(uint8_t*)0x20fa21d6 = 0;
  *(uint8_t*)0x20fa21d7 = 0;
  *(uint32_t*)0x20fa21d8 = 0;
  *(uint16_t*)0x20fa21dc = 0xf0;
  *(uint16_t*)0x20fa21de = 0x138;
  *(uint32_t*)0x20fa21e0 = 0;
  *(uint64_t*)0x20fa21e8 = 0;
  *(uint64_t*)0x20fa21f0 = 0;
  *(uint16_t*)0x20fa21f8 = 0x48;
  memcpy((void*)0x20fa21fa, "\x64\x73\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa2217 = 0;
  *(uint32_t*)0x20fa2218 = 0;
  *(uint8_t*)0x20fa221c = 0;
  *(uint8_t*)0x20fa221d = 0;
  *(uint16_t*)0x20fa221e = 0;
  *(uint16_t*)0x20fa2220 = 0;
  *(uint16_t*)0x20fa2222 = 0;
  *(uint16_t*)0x20fa2224 = 0;
  *(uint16_t*)0x20fa2226 = 0;
  *(uint16_t*)0x20fa2228 = 0;
  *(uint16_t*)0x20fa222a = 0;
  *(uint16_t*)0x20fa222c = 0;
  *(uint16_t*)0x20fa222e = 0;
  *(uint16_t*)0x20fa2230 = 0;
  *(uint16_t*)0x20fa2232 = 0;
  *(uint16_t*)0x20fa2234 = 0;
  *(uint16_t*)0x20fa2236 = 0;
  *(uint16_t*)0x20fa2238 = 0;
  *(uint16_t*)0x20fa223a = 0;
  *(uint16_t*)0x20fa223c = 0;
  *(uint8_t*)0x20fa223e = 6;
  *(uint16_t*)0x20fa2240 = 0x48;
  memcpy((void*)0x20fa2242, "\x44\x4e\x41\x54\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa225f = 1;
  *(uint32_t*)0x20fa2260 = 0;
  *(uint8_t*)0x20fa2264 = 0xfe;
  *(uint8_t*)0x20fa2265 = 0x80;
  *(uint8_t*)0x20fa2266 = 0;
  *(uint8_t*)0x20fa2267 = 0;
  *(uint8_t*)0x20fa2268 = 0;
  *(uint8_t*)0x20fa2269 = 0;
  *(uint8_t*)0x20fa226a = 0;
  *(uint8_t*)0x20fa226b = 0;
  *(uint8_t*)0x20fa226c = 0;
  *(uint8_t*)0x20fa226d = 0;
  *(uint8_t*)0x20fa226e = 0;
  *(uint8_t*)0x20fa226f = 0;
  *(uint8_t*)0x20fa2270 = 0;
  *(uint8_t*)0x20fa2271 = 0;
  *(uint8_t*)0x20fa2272 = 0;
  *(uint8_t*)0x20fa2273 = 0xbb;
  *(uint8_t*)0x20fa2274 = 0xac;
  *(uint8_t*)0x20fa2275 = 0x14;
  *(uint8_t*)0x20fa2276 = 0;
  *(uint8_t*)0x20fa2277 = 0;
  *(uint16_t*)0x20fa2284 = 0;
  *(uint16_t*)0x20fa2286 = htobe16(0x65);
  *(uint8_t*)0x20fa2288 = 0;
  *(uint8_t*)0x20fa2289 = 0;
  *(uint8_t*)0x20fa228a = 0;
  *(uint8_t*)0x20fa228b = 0;
  *(uint8_t*)0x20fa228c = 0;
  *(uint8_t*)0x20fa228d = 0;
  *(uint8_t*)0x20fa228e = 0;
  *(uint8_t*)0x20fa228f = 0;
  *(uint8_t*)0x20fa2290 = 0;
  *(uint8_t*)0x20fa2291 = 0;
  *(uint8_t*)0x20fa2292 = 0;
  *(uint8_t*)0x20fa2293 = 0;
  *(uint8_t*)0x20fa2294 = 0;
  *(uint8_t*)0x20fa2295 = 0;
  *(uint8_t*)0x20fa2296 = 0;
  *(uint8_t*)0x20fa2297 = 0;
  *(uint8_t*)0x20fa2298 = 0;
  *(uint8_t*)0x20fa2299 = 0;
  *(uint8_t*)0x20fa229a = 0;
  *(uint8_t*)0x20fa229b = 0;
  *(uint8_t*)0x20fa229c = 0;
  *(uint8_t*)0x20fa229d = 0;
  *(uint8_t*)0x20fa229e = 0;
  *(uint8_t*)0x20fa229f = 0;
  *(uint8_t*)0x20fa22a0 = 0;
  *(uint8_t*)0x20fa22a1 = 0;
  *(uint8_t*)0x20fa22a2 = 0;
  *(uint8_t*)0x20fa22a3 = 0;
  *(uint8_t*)0x20fa22a4 = 0;
  *(uint8_t*)0x20fa22a5 = 0;
  *(uint8_t*)0x20fa22a6 = 0;
  *(uint8_t*)0x20fa22a7 = 0;
  *(uint8_t*)0x20fa22a8 = 0;
  *(uint8_t*)0x20fa22a9 = 0;
  *(uint8_t*)0x20fa22aa = 0;
  *(uint8_t*)0x20fa22ab = 0;
  *(uint8_t*)0x20fa22ac = 0;
  *(uint8_t*)0x20fa22ad = 0;
  *(uint8_t*)0x20fa22ae = 0;
  *(uint8_t*)0x20fa22af = 0;
  *(uint8_t*)0x20fa22b0 = 0;
  *(uint8_t*)0x20fa22b1 = 0;
  *(uint8_t*)0x20fa22b2 = 0;
  *(uint8_t*)0x20fa22b3 = 0;
  *(uint8_t*)0x20fa22b4 = 0;
  *(uint8_t*)0x20fa22b5 = 0;
  *(uint8_t*)0x20fa22b6 = 0;
  *(uint8_t*)0x20fa22b7 = 0;
  *(uint8_t*)0x20fa22b8 = 0;
  *(uint8_t*)0x20fa22b9 = 0;
  *(uint8_t*)0x20fa22ba = 0;
  *(uint8_t*)0x20fa22bb = 0;
  *(uint8_t*)0x20fa22bc = 0;
  *(uint8_t*)0x20fa22bd = 0;
  *(uint8_t*)0x20fa22be = 0;
  *(uint8_t*)0x20fa22bf = 0;
  *(uint8_t*)0x20fa22c0 = 0;
  *(uint8_t*)0x20fa22c1 = 0;
  *(uint8_t*)0x20fa22c2 = 0;
  *(uint8_t*)0x20fa22c3 = 0;
  *(uint8_t*)0x20fa22c4 = 0;
  *(uint8_t*)0x20fa22c5 = 0;
  *(uint8_t*)0x20fa22c6 = 0;
  *(uint8_t*)0x20fa22c7 = 0;
  *(uint8_t*)0x20fa22c8 = 0;
  *(uint8_t*)0x20fa22c9 = 0;
  *(uint8_t*)0x20fa22ca = 0;
  *(uint8_t*)0x20fa22cb = 0;
  *(uint8_t*)0x20fa22cc = 0;
  *(uint8_t*)0x20fa22cd = 0;
  *(uint8_t*)0x20fa22ce = 0;
  *(uint8_t*)0x20fa22cf = 0;
  *(uint8_t*)0x20fa22d0 = 0;
  *(uint8_t*)0x20fa22d1 = 0;
  *(uint8_t*)0x20fa22d2 = 0;
  *(uint8_t*)0x20fa22d3 = 0;
  *(uint8_t*)0x20fa22d4 = 0;
  *(uint8_t*)0x20fa22d5 = 0;
  *(uint8_t*)0x20fa22d6 = 0;
  *(uint8_t*)0x20fa22d7 = 0;
  *(uint8_t*)0x20fa22d8 = 0;
  *(uint8_t*)0x20fa22d9 = 0;
  *(uint8_t*)0x20fa22da = 0;
  *(uint8_t*)0x20fa22db = 0;
  *(uint8_t*)0x20fa22dc = 0;
  *(uint8_t*)0x20fa22dd = 0;
  *(uint8_t*)0x20fa22de = 0;
  *(uint8_t*)0x20fa22df = 0;
  *(uint8_t*)0x20fa22e0 = 0;
  *(uint8_t*)0x20fa22e1 = 0;
  *(uint8_t*)0x20fa22e2 = 0;
  *(uint8_t*)0x20fa22e3 = 0;
  *(uint8_t*)0x20fa22e4 = 0;
  *(uint8_t*)0x20fa22e5 = 0;
  *(uint8_t*)0x20fa22e6 = 0;
  *(uint8_t*)0x20fa22e7 = 0;
  *(uint8_t*)0x20fa22e8 = 0;
  *(uint8_t*)0x20fa22e9 = 0;
  *(uint8_t*)0x20fa22ea = 0;
  *(uint8_t*)0x20fa22eb = 0;
  *(uint8_t*)0x20fa22ec = 0;
  *(uint8_t*)0x20fa22ed = 0;
  *(uint8_t*)0x20fa22ee = 0;
  *(uint8_t*)0x20fa22ef = 0;
  *(uint8_t*)0x20fa22f0 = 0;
  *(uint8_t*)0x20fa22f1 = 0;
  *(uint8_t*)0x20fa22f2 = 0;
  *(uint8_t*)0x20fa22f3 = 0;
  *(uint8_t*)0x20fa22f4 = 0;
  *(uint8_t*)0x20fa22f5 = 0;
  *(uint8_t*)0x20fa22f6 = 0;
  *(uint8_t*)0x20fa22f7 = 0;
  *(uint8_t*)0x20fa22f8 = 0;
  *(uint8_t*)0x20fa22f9 = 0;
  *(uint8_t*)0x20fa22fa = 0;
  *(uint8_t*)0x20fa22fb = 0;
  *(uint8_t*)0x20fa22fc = 0;
  *(uint8_t*)0x20fa22fd = 0;
  *(uint8_t*)0x20fa22fe = 0;
  *(uint8_t*)0x20fa22ff = 0;
  *(uint8_t*)0x20fa2300 = 0;
  *(uint8_t*)0x20fa2301 = 0;
  *(uint8_t*)0x20fa2302 = 0;
  *(uint8_t*)0x20fa2303 = 0;
  *(uint8_t*)0x20fa2304 = 0;
  *(uint8_t*)0x20fa2305 = 0;
  *(uint8_t*)0x20fa2306 = 0;
  *(uint8_t*)0x20fa2307 = 0;
  *(uint8_t*)0x20fa2308 = 0;
  *(uint8_t*)0x20fa2309 = 0;
  *(uint8_t*)0x20fa230a = 0;
  *(uint8_t*)0x20fa230b = 0;
  *(uint8_t*)0x20fa230c = 0;
  *(uint8_t*)0x20fa230d = 0;
  *(uint8_t*)0x20fa230e = 0;
  *(uint8_t*)0x20fa230f = 0;
  *(uint32_t*)0x20fa2310 = 0;
  *(uint16_t*)0x20fa2314 = 0xa8;
  *(uint16_t*)0x20fa2316 = 0xf0;
  *(uint32_t*)0x20fa2318 = 0;
  *(uint64_t*)0x20fa2320 = 0;
  *(uint64_t*)0x20fa2328 = 0;
  *(uint16_t*)0x20fa2330 = 0x48;
  memcpy((void*)0x20fa2332, "\x52\x45\x44\x49\x52\x45\x43\x54\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa234f = 0;
  *(uint32_t*)0x20fa2350 = 0x12;
  *(uint64_t*)0x20fa2354 = htobe64(0);
  *(uint64_t*)0x20fa235c = htobe64(1);
  *(uint8_t*)0x20fa2364 = 0xac;
  *(uint8_t*)0x20fa2365 = 0x14;
  *(uint8_t*)0x20fa2366 = 0;
  *(uint8_t*)0x20fa2367 = 0xbb;
  *(uint16_t*)0x20fa2374 = htobe16(0x4e23);
  *(uint16_t*)0x20fa2376 = htobe16(0x4e22);
  *(uint8_t*)0x20fa2378 = 0;
  *(uint8_t*)0x20fa2379 = 0;
  *(uint8_t*)0x20fa237a = 0;
  *(uint8_t*)0x20fa237b = 0;
  *(uint8_t*)0x20fa237c = 0;
  *(uint8_t*)0x20fa237d = 0;
  *(uint8_t*)0x20fa237e = 0;
  *(uint8_t*)0x20fa237f = 0;
  *(uint8_t*)0x20fa2380 = 0;
  *(uint8_t*)0x20fa2381 = 0;
  *(uint8_t*)0x20fa2382 = 0;
  *(uint8_t*)0x20fa2383 = 0;
  *(uint8_t*)0x20fa2384 = 0;
  *(uint8_t*)0x20fa2385 = 0;
  *(uint8_t*)0x20fa2386 = 0;
  *(uint8_t*)0x20fa2387 = 0;
  *(uint8_t*)0x20fa2388 = 0;
  *(uint8_t*)0x20fa2389 = 0;
  *(uint8_t*)0x20fa238a = 0;
  *(uint8_t*)0x20fa238b = 0;
  *(uint8_t*)0x20fa238c = 0;
  *(uint8_t*)0x20fa238d = 0;
  *(uint8_t*)0x20fa238e = 0;
  *(uint8_t*)0x20fa238f = 0;
  *(uint8_t*)0x20fa2390 = 0;
  *(uint8_t*)0x20fa2391 = 0;
  *(uint8_t*)0x20fa2392 = 0;
  *(uint8_t*)0x20fa2393 = 0;
  *(uint8_t*)0x20fa2394 = 0;
  *(uint8_t*)0x20fa2395 = 0;
  *(uint8_t*)0x20fa2396 = 0;
  *(uint8_t*)0x20fa2397 = 0;
  *(uint8_t*)0x20fa2398 = 0;
  *(uint8_t*)0x20fa2399 = 0;
  *(uint8_t*)0x20fa239a = 0;
  *(uint8_t*)0x20fa239b = 0;
  *(uint8_t*)0x20fa239c = 0;
  *(uint8_t*)0x20fa239d = 0;
  *(uint8_t*)0x20fa239e = 0;
  *(uint8_t*)0x20fa239f = 0;
  *(uint8_t*)0x20fa23a0 = 0;
  *(uint8_t*)0x20fa23a1 = 0;
  *(uint8_t*)0x20fa23a2 = 0;
  *(uint8_t*)0x20fa23a3 = 0;
  *(uint8_t*)0x20fa23a4 = 0;
  *(uint8_t*)0x20fa23a5 = 0;
  *(uint8_t*)0x20fa23a6 = 0;
  *(uint8_t*)0x20fa23a7 = 0;
  *(uint8_t*)0x20fa23a8 = 0;
  *(uint8_t*)0x20fa23a9 = 0;
  *(uint8_t*)0x20fa23aa = 0;
  *(uint8_t*)0x20fa23ab = 0;
  *(uint8_t*)0x20fa23ac = 0;
  *(uint8_t*)0x20fa23ad = 0;
  *(uint8_t*)0x20fa23ae = 0;
  *(uint8_t*)0x20fa23af = 0;
  *(uint8_t*)0x20fa23b0 = 0;
  *(uint8_t*)0x20fa23b1 = 0;
  *(uint8_t*)0x20fa23b2 = 0;
  *(uint8_t*)0x20fa23b3 = 0;
  *(uint8_t*)0x20fa23b4 = 0;
  *(uint8_t*)0x20fa23b5 = 0;
  *(uint8_t*)0x20fa23b6 = 0;
  *(uint8_t*)0x20fa23b7 = 0;
  *(uint8_t*)0x20fa23b8 = 0;
  *(uint8_t*)0x20fa23b9 = 0;
  *(uint8_t*)0x20fa23ba = 0;
  *(uint8_t*)0x20fa23bb = 0;
  *(uint8_t*)0x20fa23bc = 0;
  *(uint8_t*)0x20fa23bd = 0;
  *(uint8_t*)0x20fa23be = 0;
  *(uint8_t*)0x20fa23bf = 0;
  *(uint8_t*)0x20fa23c0 = 0;
  *(uint8_t*)0x20fa23c1 = 0;
  *(uint8_t*)0x20fa23c2 = 0;
  *(uint8_t*)0x20fa23c3 = 0;
  *(uint8_t*)0x20fa23c4 = 0;
  *(uint8_t*)0x20fa23c5 = 0;
  *(uint8_t*)0x20fa23c6 = 0;
  *(uint8_t*)0x20fa23c7 = 0;
  *(uint8_t*)0x20fa23c8 = 0;
  *(uint8_t*)0x20fa23c9 = 0;
  *(uint8_t*)0x20fa23ca = 0;
  *(uint8_t*)0x20fa23cb = 0;
  *(uint8_t*)0x20fa23cc = 0;
  *(uint8_t*)0x20fa23cd = 0;
  *(uint8_t*)0x20fa23ce = 0;
  *(uint8_t*)0x20fa23cf = 0;
  *(uint8_t*)0x20fa23d0 = 0;
  *(uint8_t*)0x20fa23d1 = 0;
  *(uint8_t*)0x20fa23d2 = 0;
  *(uint8_t*)0x20fa23d3 = 0;
  *(uint8_t*)0x20fa23d4 = 0;
  *(uint8_t*)0x20fa23d5 = 0;
  *(uint8_t*)0x20fa23d6 = 0;
  *(uint8_t*)0x20fa23d7 = 0;
  *(uint8_t*)0x20fa23d8 = 0;
  *(uint8_t*)0x20fa23d9 = 0;
  *(uint8_t*)0x20fa23da = 0;
  *(uint8_t*)0x20fa23db = 0;
  *(uint8_t*)0x20fa23dc = 0;
  *(uint8_t*)0x20fa23dd = 0;
  *(uint8_t*)0x20fa23de = 0;
  *(uint8_t*)0x20fa23df = 0;
  *(uint8_t*)0x20fa23e0 = 0;
  *(uint8_t*)0x20fa23e1 = 0;
  *(uint8_t*)0x20fa23e2 = 0;
  *(uint8_t*)0x20fa23e3 = 0;
  *(uint8_t*)0x20fa23e4 = 0;
  *(uint8_t*)0x20fa23e5 = 0;
  *(uint8_t*)0x20fa23e6 = 0;
  *(uint8_t*)0x20fa23e7 = 0;
  *(uint8_t*)0x20fa23e8 = 0;
  *(uint8_t*)0x20fa23e9 = 0;
  *(uint8_t*)0x20fa23ea = 0;
  *(uint8_t*)0x20fa23eb = 0;
  *(uint8_t*)0x20fa23ec = 0;
  *(uint8_t*)0x20fa23ed = 0;
  *(uint8_t*)0x20fa23ee = 0;
  *(uint8_t*)0x20fa23ef = 0;
  *(uint8_t*)0x20fa23f0 = 0;
  *(uint8_t*)0x20fa23f1 = 0;
  *(uint8_t*)0x20fa23f2 = 0;
  *(uint8_t*)0x20fa23f3 = 0;
  *(uint8_t*)0x20fa23f4 = 0;
  *(uint8_t*)0x20fa23f5 = 0;
  *(uint8_t*)0x20fa23f6 = 0;
  *(uint8_t*)0x20fa23f7 = 0;
  *(uint8_t*)0x20fa23f8 = 0;
  *(uint8_t*)0x20fa23f9 = 0;
  *(uint8_t*)0x20fa23fa = 0;
  *(uint8_t*)0x20fa23fb = 0;
  *(uint8_t*)0x20fa23fc = 0;
  *(uint8_t*)0x20fa23fd = 0;
  *(uint8_t*)0x20fa23fe = 0;
  *(uint8_t*)0x20fa23ff = 0;
  *(uint32_t*)0x20fa2400 = 0;
  *(uint16_t*)0x20fa2404 = 0xa8;
  *(uint16_t*)0x20fa2406 = 0xf0;
  *(uint32_t*)0x20fa2408 = 0;
  *(uint64_t*)0x20fa2410 = 0;
  *(uint64_t*)0x20fa2418 = 0;
  *(uint16_t*)0x20fa2420 = 0x48;
  memcpy((void*)0x20fa2422, "\x4e\x45\x54\x4d\x41\x50\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa243f = 0;
  *(uint32_t*)0x20fa2440 = 1;
  *(uint32_t*)0x20fa2444 = htobe32(-1);
  *(uint8_t*)0x20fa2454 = 0xac;
  *(uint8_t*)0x20fa2455 = 0x14;
  *(uint8_t*)0x20fa2456 = 0;
  *(uint8_t*)0x20fa2457 = 0xaa;
  *(uint16_t*)0x20fa2464 = 0;
  *(uint16_t*)0x20fa2466 = 0;
  *(uint8_t*)0x20fa2468 = 0;
  *(uint8_t*)0x20fa2469 = 0;
  *(uint8_t*)0x20fa246a = 0;
  *(uint8_t*)0x20fa246b = 0;
  *(uint8_t*)0x20fa246c = 0;
  *(uint8_t*)0x20fa246d = 0;
  *(uint8_t*)0x20fa246e = 0;
  *(uint8_t*)0x20fa246f = 0;
  *(uint8_t*)0x20fa2470 = 0;
  *(uint8_t*)0x20fa2471 = 0;
  *(uint8_t*)0x20fa2472 = 0;
  *(uint8_t*)0x20fa2473 = 0;
  *(uint8_t*)0x20fa2474 = 0;
  *(uint8_t*)0x20fa2475 = 0;
  *(uint8_t*)0x20fa2476 = 0;
  *(uint8_t*)0x20fa2477 = 0;
  *(uint8_t*)0x20fa2478 = 0;
  *(uint8_t*)0x20fa2479 = 0;
  *(uint8_t*)0x20fa247a = 0;
  *(uint8_t*)0x20fa247b = 0;
  *(uint8_t*)0x20fa247c = 0;
  *(uint8_t*)0x20fa247d = 0;
  *(uint8_t*)0x20fa247e = 0;
  *(uint8_t*)0x20fa247f = 0;
  *(uint8_t*)0x20fa2480 = 0;
  *(uint8_t*)0x20fa2481 = 0;
  *(uint8_t*)0x20fa2482 = 0;
  *(uint8_t*)0x20fa2483 = 0;
  *(uint8_t*)0x20fa2484 = 0;
  *(uint8_t*)0x20fa2485 = 0;
  *(uint8_t*)0x20fa2486 = 0;
  *(uint8_t*)0x20fa2487 = 0;
  *(uint8_t*)0x20fa2488 = 0;
  *(uint8_t*)0x20fa2489 = 0;
  *(uint8_t*)0x20fa248a = 0;
  *(uint8_t*)0x20fa248b = 0;
  *(uint8_t*)0x20fa248c = 0;
  *(uint8_t*)0x20fa248d = 0;
  *(uint8_t*)0x20fa248e = 0;
  *(uint8_t*)0x20fa248f = 0;
  *(uint8_t*)0x20fa2490 = 0;
  *(uint8_t*)0x20fa2491 = 0;
  *(uint8_t*)0x20fa2492 = 0;
  *(uint8_t*)0x20fa2493 = 0;
  *(uint8_t*)0x20fa2494 = 0;
  *(uint8_t*)0x20fa2495 = 0;
  *(uint8_t*)0x20fa2496 = 0;
  *(uint8_t*)0x20fa2497 = 0;
  *(uint8_t*)0x20fa2498 = 0;
  *(uint8_t*)0x20fa2499 = 0;
  *(uint8_t*)0x20fa249a = 0;
  *(uint8_t*)0x20fa249b = 0;
  *(uint8_t*)0x20fa249c = 0;
  *(uint8_t*)0x20fa249d = 0;
  *(uint8_t*)0x20fa249e = 0;
  *(uint8_t*)0x20fa249f = 0;
  *(uint8_t*)0x20fa24a0 = 0;
  *(uint8_t*)0x20fa24a1 = 0;
  *(uint8_t*)0x20fa24a2 = 0;
  *(uint8_t*)0x20fa24a3 = 0;
  *(uint8_t*)0x20fa24a4 = 0;
  *(uint8_t*)0x20fa24a5 = 0;
  *(uint8_t*)0x20fa24a6 = 0;
  *(uint8_t*)0x20fa24a7 = 0;
  *(uint8_t*)0x20fa24a8 = 0;
  *(uint8_t*)0x20fa24a9 = 0;
  *(uint8_t*)0x20fa24aa = 0;
  *(uint8_t*)0x20fa24ab = 0;
  *(uint8_t*)0x20fa24ac = 0;
  *(uint8_t*)0x20fa24ad = 0;
  *(uint8_t*)0x20fa24ae = 0;
  *(uint8_t*)0x20fa24af = 0;
  *(uint8_t*)0x20fa24b0 = 0;
  *(uint8_t*)0x20fa24b1 = 0;
  *(uint8_t*)0x20fa24b2 = 0;
  *(uint8_t*)0x20fa24b3 = 0;
  *(uint8_t*)0x20fa24b4 = 0;
  *(uint8_t*)0x20fa24b5 = 0;
  *(uint8_t*)0x20fa24b6 = 0;
  *(uint8_t*)0x20fa24b7 = 0;
  *(uint8_t*)0x20fa24b8 = 0;
  *(uint8_t*)0x20fa24b9 = 0;
  *(uint8_t*)0x20fa24ba = 0;
  *(uint8_t*)0x20fa24bb = 0;
  *(uint8_t*)0x20fa24bc = 0;
  *(uint8_t*)0x20fa24bd = 0;
  *(uint8_t*)0x20fa24be = 0;
  *(uint8_t*)0x20fa24bf = 0;
  *(uint8_t*)0x20fa24c0 = 0;
  *(uint8_t*)0x20fa24c1 = 0;
  *(uint8_t*)0x20fa24c2 = 0;
  *(uint8_t*)0x20fa24c3 = 0;
  *(uint8_t*)0x20fa24c4 = 0;
  *(uint8_t*)0x20fa24c5 = 0;
  *(uint8_t*)0x20fa24c6 = 0;
  *(uint8_t*)0x20fa24c7 = 0;
  *(uint8_t*)0x20fa24c8 = 0;
  *(uint8_t*)0x20fa24c9 = 0;
  *(uint8_t*)0x20fa24ca = 0;
  *(uint8_t*)0x20fa24cb = 0;
  *(uint8_t*)0x20fa24cc = 0;
  *(uint8_t*)0x20fa24cd = 0;
  *(uint8_t*)0x20fa24ce = 0;
  *(uint8_t*)0x20fa24cf = 0;
  *(uint8_t*)0x20fa24d0 = 0;
  *(uint8_t*)0x20fa24d1 = 0;
  *(uint8_t*)0x20fa24d2 = 0;
  *(uint8_t*)0x20fa24d3 = 0;
  *(uint8_t*)0x20fa24d4 = 0;
  *(uint8_t*)0x20fa24d5 = 0;
  *(uint8_t*)0x20fa24d6 = 0;
  *(uint8_t*)0x20fa24d7 = 0;
  *(uint8_t*)0x20fa24d8 = 0;
  *(uint8_t*)0x20fa24d9 = 0;
  *(uint8_t*)0x20fa24da = 0;
  *(uint8_t*)0x20fa24db = 0;
  *(uint8_t*)0x20fa24dc = 0;
  *(uint8_t*)0x20fa24dd = 0;
  *(uint8_t*)0x20fa24de = 0;
  *(uint8_t*)0x20fa24df = 0;
  *(uint8_t*)0x20fa24e0 = 0;
  *(uint8_t*)0x20fa24e1 = 0;
  *(uint8_t*)0x20fa24e2 = 0;
  *(uint8_t*)0x20fa24e3 = 0;
  *(uint8_t*)0x20fa24e4 = 0;
  *(uint8_t*)0x20fa24e5 = 0;
  *(uint8_t*)0x20fa24e6 = 0;
  *(uint8_t*)0x20fa24e7 = 0;
  *(uint8_t*)0x20fa24e8 = 0;
  *(uint8_t*)0x20fa24e9 = 0;
  *(uint8_t*)0x20fa24ea = 0;
  *(uint8_t*)0x20fa24eb = 0;
  *(uint8_t*)0x20fa24ec = 0;
  *(uint8_t*)0x20fa24ed = 0;
  *(uint8_t*)0x20fa24ee = 0;
  *(uint8_t*)0x20fa24ef = 0;
  *(uint32_t*)0x20fa24f0 = 0;
  *(uint16_t*)0x20fa24f4 = 0xa8;
  *(uint16_t*)0x20fa24f6 = 0xd0;
  *(uint32_t*)0x20fa24f8 = 0;
  *(uint64_t*)0x20fa2500 = 0;
  *(uint64_t*)0x20fa2508 = 0;
  *(uint16_t*)0x20fa2510 = 0x28;
  memcpy((void*)0x20fa2512, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa252f = 0;
  *(uint32_t*)0x20fa2530 = 0xfffffffe;
  syscall(__NR_setsockopt, r[0], 0x29, 0x40, 0x20fa2000, 0x538);
  *(uint16_t*)0x208fdfe4 = 0xa;
  *(uint16_t*)0x208fdfe6 = htobe16(0x4e22);
  *(uint32_t*)0x208fdfe8 = 0;
  *(uint8_t*)0x208fdfec = 0;
  *(uint8_t*)0x208fdfed = 0;
  *(uint8_t*)0x208fdfee = 0;
  *(uint8_t*)0x208fdfef = 0;
  *(uint8_t*)0x208fdff0 = 0;
  *(uint8_t*)0x208fdff1 = 0;
  *(uint8_t*)0x208fdff2 = 0;
  *(uint8_t*)0x208fdff3 = 0;
  *(uint8_t*)0x208fdff4 = 0;
  *(uint8_t*)0x208fdff5 = 0;
  *(uint8_t*)0x208fdff6 = 0;
  *(uint8_t*)0x208fdff7 = 0;
  *(uint8_t*)0x208fdff8 = 0;
  *(uint8_t*)0x208fdff9 = 0;
  *(uint8_t*)0x208fdffa = 0;
  *(uint8_t*)0x208fdffb = 0;
  *(uint32_t*)0x208fdffc = 0;
  syscall(__NR_connect, r[0], 0x208fdfe4, 0x1c);
  *(uint8_t*)0x20013ed8 = 0xaa;
  *(uint8_t*)0x20013ed9 = 0xaa;
  *(uint8_t*)0x20013eda = 0xaa;
  *(uint8_t*)0x20013edb = 0xaa;
  *(uint8_t*)0x20013edc = 0;
  *(uint8_t*)0x20013edd = 0x14;
  *(uint8_t*)0x20013ede = 1;
  *(uint8_t*)0x20013edf = 0x80;
  *(uint8_t*)0x20013ee0 = 0xc2;
  *(uint8_t*)0x20013ee1 = 0;
  *(uint8_t*)0x20013ee2 = 0;
  *(uint8_t*)0x20013ee3 = 0xf;
  *(uint16_t*)0x20013ee4 = htobe16(0x9100);
  STORE_BY_BITMASK(uint16_t, 0x20013ee6, 0xfeff, 0, 3);
  STORE_BY_BITMASK(uint16_t, 0x20013ee6, 0x101, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20013ee6, 4, 4, 12);
  *(uint16_t*)0x20013ee8 = htobe16(0x8100);
  STORE_BY_BITMASK(uint16_t, 0x20013eea, 5, 0, 3);
  STORE_BY_BITMASK(uint16_t, 0x20013eea, 9, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20013eea, 0x706, 4, 12);
  *(uint16_t*)0x20013eec = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x20013eee, 0x11, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20013eee, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20013eef, 0x40, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20013eef, 0x80, 2, 6);
  *(uint16_t*)0x20013ef0 = htobe16(0x54);
  *(uint16_t*)0x20013ef2 = htobe16(0x66);
  *(uint16_t*)0x20013ef4 = htobe16(5);
  *(uint8_t*)0x20013ef6 = 3;
  *(uint8_t*)0x20013ef7 = 5;
  *(uint16_t*)0x20013ef8 = 0;
  *(uint32_t*)0x20013efa = htobe32(0);
  *(uint32_t*)0x20013efe = htobe32(-1);
  *(uint8_t*)0x20013f02 = 0;
  *(uint8_t*)0x20013f03 = 0x94;
  *(uint8_t*)0x20013f04 = 6;
  *(uint32_t*)0x20013f05 = htobe32(4);
  *(uint8_t*)0x20013f09 = 0x86;
  *(uint8_t*)0x20013f0a = 0x12;
  *(uint32_t*)0x20013f0b = htobe32(7);
  *(uint8_t*)0x20013f0f = 6;
  *(uint8_t*)0x20013f10 = 2;
  *(uint8_t*)0x20013f11 = 2;
  *(uint8_t*)0x20013f12 = 2;
  *(uint8_t*)0x20013f13 = 5;
  *(uint8_t*)0x20013f14 = 2;
  *(uint8_t*)0x20013f15 = 5;
  *(uint8_t*)0x20013f16 = 2;
  *(uint8_t*)0x20013f17 = 1;
  *(uint8_t*)0x20013f18 = 2;
  *(uint8_t*)0x20013f19 = 5;
  *(uint8_t*)0x20013f1a = 2;
  *(uint8_t*)0x20013f1b = 0x94;
  *(uint8_t*)0x20013f1c = 6;
  *(uint32_t*)0x20013f1d = htobe32(0xaa);
  *(uint8_t*)0x20013f21 = 7;
  *(uint8_t*)0x20013f22 = 0xb;
  *(uint8_t*)0x20013f23 = 0;
  *(uint8_t*)0x20013f24 = 0xac;
  *(uint8_t*)0x20013f25 = 0x14;
  *(uint8_t*)0x20013f26 = 0;
  *(uint8_t*)0x20013f27 = 0xbb;
  *(uint8_t*)0x20013f28 = 0xac;
  *(uint8_t*)0x20013f29 = 0x14;
  *(uint8_t*)0x20013f2a = 0;
  *(uint8_t*)0x20013f2b = 0x17;
  *(uint8_t*)0x20013f2c = 0x94;
  *(uint8_t*)0x20013f2d = 6;
  *(uint32_t*)0x20013f2e = htobe32(2);
  *(uint16_t*)0x20013f32 = htobe16(0x4e22);
  *(uint16_t*)0x20013f34 = htobe16(0x4e20);
  *(uint8_t*)0x20013f36 = 4;
  STORE_BY_BITMASK(uint8_t, 0x20013f37, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20013f37, 0xfe, 4, 4);
  *(uint16_t*)0x20013f38 = 0;
  STORE_BY_BITMASK(uint8_t, 0x20013f3a, 0, 0, 1);
  STORE_BY_BITMASK(uint8_t, 0x20013f3a, 7, 1, 4);
  STORE_BY_BITMASK(uint8_t, 0x20013f3a, 2, 5, 3);
  memcpy((void*)0x20013f3b, "\x6d\xb1\x02", 3);
  *(uint8_t*)0x20013f3e = 0x7f;
  memcpy((void*)0x20013f3f, "\xea\xab\xbd", 3);
  *(uint32_t*)0x20013ff0 = 1;
  *(uint32_t*)0x20013ff4 = 2;
  *(uint32_t*)0x20013ff8 = 0x48a;
  *(uint32_t*)0x20013ffc = 0x41;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20013efa, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x20013efe, 4);
  uint16_t csum_1_chunk_2 = 0x2100;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 2);
  uint16_t csum_1_chunk_3 = 0x1000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 2);
  csum_inet_update(&csum_1, (const uint8_t*)0x20013f32, 16);
  *(uint16_t*)0x20013f38 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x20013eee, 68);
  *(uint16_t*)0x20013ef8 = csum_inet_digest(&csum_2);
}

int main()
{
  loop();
  return 0;
}
