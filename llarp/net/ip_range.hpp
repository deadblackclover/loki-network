#pragma once
#include <ostream>
#include <net/ip.hpp>
#include <net/net_bits.hpp>
#include <util/bits.hpp>
#include <util/types.hpp>
#include <string>

namespace llarp
{
  struct IPRange
  {
    using Addr_t = huint128_t;
    huint128_t addr = {0};
    huint128_t netmask_bits = {0};

    static constexpr IPRange
    FromIPv4(byte_t a, byte_t b, byte_t c, byte_t d, byte_t mask)
    {
      return IPRange{net::ExpandV4(ipaddr_ipv4_bits(a, b, c, d)), netmask_ipv6_bits(mask + 96)};
    }

    /// return true if this iprange is in the SIIT range for containing ipv4 addresses
    constexpr bool
    IsV4() const
    {
      constexpr auto siit = IPRange{huint128_t{0x0000'ffff'0000'0000UL}, netmask_ipv6_bits(96)};
      return siit.Contains(addr);
    }

    /// return the number of bits set in the hostmask
    constexpr int
    HostmaskBits() const
    {
      if (IsV4())
      {
        return bits::count_bits(net::TruncateV6(netmask_bits));
      }
      return bits::count_bits(netmask_bits);
    }

    /// return true if the other range is inside our range
    constexpr bool
    Contains(const IPRange& other) const
    {
      return Contains(other.addr) and Contains(other.HighestAddr());
    }

    /// return true if ip is contained in this ip range
    constexpr bool
    Contains(const Addr_t& ip) const
    {
      return (addr & netmask_bits) == (ip & netmask_bits);
    }

    /// return true if we are a ipv4 range and contains this ip
    constexpr bool
    Contains(const huint32_t& ip) const
    {
      if (not IsV4())
        return false;
      return Contains(net::ExpandV4(ip));
    }

    friend std::ostream&
    operator<<(std::ostream& out, const IPRange& a)
    {
      return out << a.ToString();
    }

    /// get the highest address on this range
    constexpr huint128_t
    HighestAddr() const
    {
      return (addr & netmask_bits) + (huint128_t{1} << (128 - bits::count_bits_128(netmask_bits.h)))
          - huint128_t{1};
    }

    bool
    operator<(const IPRange& other) const
    {
      return (this->addr & this->netmask_bits) < (other.addr & other.netmask_bits)
          || this->netmask_bits < other.netmask_bits;
    }

    std::string
    ToString() const
    {
      return BaseAddressString() + "/" + std::to_string(HostmaskBits());
    }

    std::string
    BaseAddressString() const;

    bool
    FromString(std::string str);
  };

}  // namespace llarp
