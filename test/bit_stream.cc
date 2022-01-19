#include "gtest/gtest.h"
#include "stream/bit_stream.h"

using BitStream = stream::BitStream;

namespace {

  TEST(BitStreamTest, AllocateSingleBlock) {
    BitStream uut(63);

    ASSERT_EQ(uut.block_size(), 1);
    ASSERT_EQ(uut.size(), 64);
  }

  TEST(BitStreamTest, AllocateMultipleBlocks) {
    BitStream uut(64);

    ASSERT_EQ(uut.block_size(), 2);
    ASSERT_EQ(uut.size(), 128);
  }

  TEST(BitStreamTest, ShiftRightWithCarry) {
    BitStream uut(128);

    ASSERT_EQ(uut.block_size(), 3);

    uut.set(63, 1);
    uut.set(127, 1);

    ASSERT_TRUE(uut.is_set(63));
    ASSERT_TRUE(uut.is_set(127));
    
    uut >>= 1;

    ASSERT_FALSE(uut.is_set(63));
    ASSERT_FALSE(uut.is_set(127));

    ASSERT_TRUE(uut.is_set(64));
    ASSERT_TRUE(uut.is_set(128));
  }

  TEST(BitStreamTest, AdditionSingleBlock) {
    BitStream a(10);
    BitStream b(20);

    a.set(50, 1);
    a.set(51, 1);
    a.set(52, 1);

    b.set(51, 1);
    b.set(52, 1);
    b.set(53, 1);

    auto c = a + b;

    ASSERT_TRUE(c.is_set(50));
    ASSERT_FALSE(c.is_set(51));
    ASSERT_TRUE(c.is_set(52));
    ASSERT_FALSE(c.is_set(53));
    ASSERT_TRUE(c.is_set(54));
  }

  TEST(BitStreamTest, AdditionMultipleBlock) {
    BitStream a(80);
    BitStream b(90);

    a.set(62, 1);
    a.set(63, 1);
    a.set(64, 1);

    b.set(63, 1);
    b.set(64, 1);
    b.set(65, 1);

    auto c = a + b;

    ASSERT_TRUE(c.is_set(62));
    ASSERT_FALSE(c.is_set(63));
    ASSERT_TRUE(c.is_set(64));
    ASSERT_FALSE(c.is_set(65));
    ASSERT_TRUE(c.is_set(66));
  }

  TEST(BitStreamTest, AdditionMultipleBlockEndLimit) {
    BitStream a(80);
    BitStream b(90);

    a.set(61, 1);
    a.set(62, 1);
    a.set(63, 1);

    b.set(62, 1);
    b.set(63, 1);
    b.set(64, 1);

    auto c = a + b;

    ASSERT_TRUE(c.is_set(61));
    ASSERT_FALSE(c.is_set(62));
    ASSERT_TRUE(c.is_set(63));
    ASSERT_FALSE(c.is_set(64));
    ASSERT_TRUE(c.is_set(65));
  }

  TEST(BitStreamTest, AdditionTwoBlocksAllOnes) {
    BitStream a(80);
    BitStream b(90);

    for (auto i = 0; i < 63; ++i) {
      a.set(i, 1);
      b.set(i, 1);
    }

    auto c = a + b;

    ASSERT_FALSE(c.is_set(0));
    for (auto i = 1; i < 64; ++i) {
      ASSERT_TRUE(c.is_set(i));
    }
  }

  TEST(BitStreamTest, PopCountSingleBlock) {
    BitStream a(10);

    for (auto i = 0; i < 63; ++i) {
      a.set(i, i % 2);
    }

    ASSERT_EQ(a.pop_count(), 31);
  }
  
  TEST(BitStreamTest, PopCountMultipleBlocks) {
    BitStream a(80);

    auto expected_count = 0;
    for (auto i = 0; i < 126; ++i) {
      expected_count += i % 2;
      a.set(i, i % 2);
    }

    ASSERT_EQ(a.pop_count(), expected_count);
  }

  TEST(BitStreamTest, PopCountMultipleBlocksRandomSet) {
    BitStream a(80);

    auto expected_count = 0;
    for (auto i = 0; i < 126; ++i) {
      auto is_one = rand() % 2;
      expected_count += is_one;
      a.set(i, is_one);
    }

    ASSERT_EQ(a.pop_count(), expected_count);
  }

} // namespace
