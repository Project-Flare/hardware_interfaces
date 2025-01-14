/*
 * Copyright 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <aidl/android/hardware/security/keymint/RpcHardwareInfo.h>
#include <android-base/properties.h>
#include <cppbor.h>
#include <cppbor_parse.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <keymaster/android_keymaster_utils.h>
#include <keymaster/cppcose/cppcose.h>
#include <keymaster/logger.h>
#include <keymaster/remote_provisioning_utils.h>
#include <openssl/curve25519.h>
#include <remote_prov/remote_prov_utils.h>

#include <algorithm>
#include <cstdint>
#include <span>

namespace aidl::android::hardware::security::keymint::remote_prov {
namespace {

using ::keymaster::KeymasterBlob;
using ::keymaster::kStatusFailed;
using ::keymaster::kStatusInvalidEek;
using ::keymaster::StatusOr;
using ::testing::ElementsAreArray;
using byte_view = std::span<const uint8_t>;

inline const std::vector<uint8_t> kDegenerateBcc{
        0x82, 0xa5, 0x01, 0x01, 0x03, 0x27, 0x04, 0x81, 0x02, 0x20, 0x06, 0x21, 0x58, 0x20, 0xf5,
        0x5a, 0xfb, 0x28, 0x06, 0x48, 0x68, 0xea, 0x49, 0x3e, 0x47, 0x80, 0x1d, 0xfe, 0x1f, 0xfc,
        0xa8, 0x84, 0xb3, 0x4d, 0xdb, 0x99, 0xc7, 0xbf, 0x23, 0x53, 0xe5, 0x71, 0x92, 0x41, 0x9b,
        0x46, 0x84, 0x43, 0xa1, 0x01, 0x27, 0xa0, 0x59, 0x01, 0x75, 0xa9, 0x01, 0x78, 0x28, 0x31,
        0x64, 0x34, 0x35, 0x65, 0x33, 0x35, 0x63, 0x34, 0x35, 0x37, 0x33, 0x31, 0x61, 0x32, 0x62,
        0x34, 0x35, 0x36, 0x37, 0x63, 0x33, 0x63, 0x65, 0x35, 0x31, 0x36, 0x66, 0x35, 0x63, 0x31,
        0x66, 0x34, 0x33, 0x61, 0x62, 0x64, 0x36, 0x30, 0x62, 0x02, 0x78, 0x28, 0x31, 0x64, 0x34,
        0x35, 0x65, 0x33, 0x35, 0x63, 0x34, 0x35, 0x37, 0x33, 0x31, 0x61, 0x32, 0x62, 0x34, 0x35,
        0x36, 0x37, 0x63, 0x33, 0x63, 0x65, 0x35, 0x31, 0x36, 0x66, 0x35, 0x63, 0x31, 0x66, 0x34,
        0x33, 0x61, 0x62, 0x64, 0x36, 0x30, 0x62, 0x3a, 0x00, 0x47, 0x44, 0x50, 0x58, 0x40, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x3a, 0x00, 0x47, 0x44, 0x53, 0x41, 0xa0, 0x3a, 0x00, 0x47, 0x44, 0x52,
        0x58, 0x40, 0x71, 0xd7, 0x47, 0x9e, 0x61, 0xb5, 0x30, 0xa3, 0xda, 0xe6, 0xac, 0xb2, 0x91,
        0xa4, 0xf9, 0xcf, 0x7f, 0xba, 0x6b, 0x5f, 0xf9, 0xa3, 0x7f, 0xba, 0xab, 0xac, 0x69, 0xdd,
        0x0b, 0x04, 0xd6, 0x34, 0xd2, 0x3f, 0x8f, 0x84, 0x96, 0xd7, 0x58, 0x51, 0x1d, 0x68, 0x25,
        0xea, 0xbe, 0x11, 0x11, 0x1e, 0xd8, 0xdf, 0x4b, 0x62, 0x78, 0x5c, 0xa8, 0xfa, 0xb7, 0x66,
        0x4e, 0x8d, 0xac, 0x3b, 0x00, 0x4c, 0x3a, 0x00, 0x47, 0x44, 0x54, 0x58, 0x40, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x3a, 0x00, 0x47, 0x44, 0x56, 0x41, 0x00, 0x3a, 0x00, 0x47, 0x44, 0x57, 0x58,
        0x2d, 0xa5, 0x01, 0x01, 0x03, 0x27, 0x04, 0x81, 0x02, 0x20, 0x06, 0x21, 0x58, 0x20, 0xf5,
        0x5a, 0xfb, 0x28, 0x06, 0x48, 0x68, 0xea, 0x49, 0x3e, 0x47, 0x80, 0x1d, 0xfe, 0x1f, 0xfc,
        0xa8, 0x84, 0xb3, 0x4d, 0xdb, 0x99, 0xc7, 0xbf, 0x23, 0x53, 0xe5, 0x71, 0x92, 0x41, 0x9b,
        0x46, 0x3a, 0x00, 0x47, 0x44, 0x58, 0x41, 0x20, 0x58, 0x40, 0x31, 0x5c, 0x43, 0x87, 0xf9,
        0xbb, 0xb9, 0x45, 0x09, 0xa8, 0xe8, 0x08, 0x70, 0xc4, 0xd9, 0xdc, 0x4e, 0x5a, 0x19, 0x10,
        0x4f, 0xa3, 0x21, 0x20, 0x34, 0x05, 0x5b, 0x2e, 0x78, 0x91, 0xd1, 0xe2, 0x39, 0x43, 0x8b,
        0x50, 0x12, 0x82, 0x37, 0xfe, 0xa4, 0x07, 0xc3, 0xd5, 0xc3, 0x78, 0xcc, 0xf9, 0xef, 0xe1,
        0x95, 0x38, 0x9f, 0xb0, 0x79, 0x16, 0x4c, 0x4a, 0x23, 0xc4, 0xdc, 0x35, 0x4e, 0x0f};

inline bool equal_byte_views(const byte_view& view1, const byte_view& view2) {
    return std::equal(view1.begin(), view1.end(), view2.begin(), view2.end());
}

struct KeyInfoEcdsa {
    CoseKeyCurve curve;
    byte_view pubKeyX;
    byte_view pubKeyY;

    bool operator==(const KeyInfoEcdsa& other) const {
        return curve == other.curve && equal_byte_views(pubKeyX, other.pubKeyX) &&
               equal_byte_views(pubKeyY, other.pubKeyY);
    }
};

// The production root signing key for Google ECDSA P256 Endpoint Encryption Key cert chains.
inline constexpr uint8_t kEcdsa256GeekRootX[] = {
    0xf7, 0x14, 0x8a, 0xdb, 0x97, 0xf4, 0xcc, 0x53, 0xef, 0xd2, 0x64, 0x11, 0xc4, 0xe3, 0x75, 0x1f,
    0x66, 0x1f, 0xa4, 0x71, 0x0c, 0x6c, 0xcf, 0xfa, 0x09, 0x46, 0x80, 0x74, 0x87, 0x54, 0xf2, 0xad};

inline constexpr uint8_t kEcdsa256GeekRootY[] = {
    0x5e, 0x7f, 0x5b, 0xf6, 0xec, 0xe4, 0xf6, 0x19, 0xcc, 0xff, 0x13, 0x37, 0xfd, 0x0f, 0xa1, 0xc8,
    0x93, 0xdb, 0x18, 0x06, 0x76, 0xc4, 0x5d, 0xe6, 0xd7, 0x6a, 0x77, 0x86, 0xc3, 0x2d, 0xaf, 0x8f};

// Hard-coded set of acceptable public COSE_Keys that can act as roots of EEK chains.
inline constexpr KeyInfoEcdsa kAuthorizedEcdsa256EekRoots[] = {
    {CoseKeyCurve::P256, byte_view(kEcdsa256GeekRootX, sizeof(kEcdsa256GeekRootX)),
     byte_view(kEcdsa256GeekRootY, sizeof(kEcdsa256GeekRootY))},
};

static ErrMsgOr<CoseKey> parseEcdh256(const bytevec& coseKey) {
    auto key = CoseKey::parse(coseKey, EC2, ECDH_ES_HKDF_256, P256);
    if (!key) return key;

    auto& pubkey_x = key->getMap().get(cppcose::CoseKey::PUBKEY_X);
    auto& pubkey_y = key->getMap().get(cppcose::CoseKey::PUBKEY_Y);
    if (!pubkey_x || !pubkey_y || !pubkey_x->asBstr() || !pubkey_y->asBstr() ||
        pubkey_x->asBstr()->value().size() != 32 || pubkey_y->asBstr()->value().size() != 32) {
        return "Invalid P256 public key";
    }

    return key;
}

StatusOr<std::tuple<std::vector<uint8_t> /* EEK pubX */, std::vector<uint8_t> /* EEK pubY */,
                    std::vector<uint8_t> /* EEK ID */>>
validateAndExtractEcdsa256EekPubAndId(bool testMode,
                                      const KeymasterBlob& endpointEncryptionCertChain) {
    auto [item, newPos, errMsg] =
        cppbor::parse(endpointEncryptionCertChain.begin(), endpointEncryptionCertChain.end());
    if (!item || !item->asArray()) {
        return kStatusFailed;
    }
    const cppbor::Array* certArr = item->asArray();
    std::vector<uint8_t> lastPubKey;
    for (size_t i = 0; i < certArr->size(); ++i) {
        auto cosePubKey =
            verifyAndParseCoseSign1(certArr->get(i)->asArray(), lastPubKey, {} /* AAD */);
        if (!cosePubKey) {
            return kStatusInvalidEek;
        }
        lastPubKey = *std::move(cosePubKey);

        // In prod mode the first pubkey should match a well-known Google public key.
        if (!testMode && i == 0) {
            auto parsedPubKey = CoseKey::parse(lastPubKey);
            if (!parsedPubKey) {
                return kStatusFailed;
            }
            auto curve = parsedPubKey->getIntValue(CoseKey::CURVE);
            if (!curve) {
                return kStatusInvalidEek;
            }
            auto rawPubX = parsedPubKey->getBstrValue(CoseKey::PUBKEY_X);
            if (!rawPubX) {
                return kStatusInvalidEek;
            }
            auto rawPubY = parsedPubKey->getBstrValue(CoseKey::PUBKEY_Y);
            if (!rawPubY) {
                return kStatusInvalidEek;
            }
            KeyInfoEcdsa matcher = {static_cast<CoseKeyCurve>(*curve),
                                    byte_view(rawPubX->data(), rawPubX->size()),
                                    byte_view(rawPubY->data(), rawPubY->size())};
            if (std::find(std::begin(kAuthorizedEcdsa256EekRoots),
                          std::end(kAuthorizedEcdsa256EekRoots),
                          matcher) == std::end(kAuthorizedEcdsa256EekRoots)) {
                return kStatusInvalidEek;
            }
        }
    }
    auto eek = parseEcdh256(lastPubKey);
    if (!eek) {
        return kStatusInvalidEek;
    }
    return std::make_tuple(eek->getBstrValue(CoseKey::PUBKEY_X).value(),
                           eek->getBstrValue(CoseKey::PUBKEY_Y).value(),
                           eek->getBstrValue(CoseKey::KEY_ID).value());
}

TEST(RemoteProvUtilsTest, GenerateEekChainInvalidLength) {
    ASSERT_FALSE(generateEekChain(RpcHardwareInfo::CURVE_25519, 1, /*eekId=*/{}));
}

TEST(RemoteProvUtilsTest, GenerateEekChain) {
    bytevec kTestEekId = {'t', 'e', 's', 't', 'I', 'd', 0};
    for (size_t length : {2, 3, 31}) {
        auto get_eek_result = generateEekChain(RpcHardwareInfo::CURVE_25519, length, kTestEekId);
        ASSERT_TRUE(get_eek_result) << get_eek_result.message();

        auto& [chain, pubkey, privkey] = *get_eek_result;

        auto validation_result = validateAndExtractEekPubAndId(
                /*testMode=*/true, KeymasterBlob(chain.data(), chain.size()));
        ASSERT_TRUE(validation_result.isOk());

        auto& [eekPub, eekId] = *validation_result;
        EXPECT_THAT(eekId, ElementsAreArray(kTestEekId));
        EXPECT_THAT(eekPub, ElementsAreArray(pubkey));
    }
}

TEST(RemoteProvUtilsTest, GetProdEekChain) {
    auto chain = getProdEekChain(RpcHardwareInfo::CURVE_25519);

    auto validation_result = validateAndExtractEekPubAndId(
            /*testMode=*/false, KeymasterBlob(chain.data(), chain.size()));
    ASSERT_TRUE(validation_result.isOk()) << "Error: " << validation_result.moveError();

    auto& [eekPub, eekId] = *validation_result;

    auto [geekCert, ignoredNewPos, error] =
            cppbor::parse(kCoseEncodedGeekCert, sizeof(kCoseEncodedGeekCert));
    ASSERT_NE(geekCert, nullptr) << "Error: " << error;
    ASSERT_NE(geekCert->asArray(), nullptr);

    auto& encodedGeekCoseKey = geekCert->asArray()->get(kCoseSign1Payload);
    ASSERT_NE(encodedGeekCoseKey, nullptr);
    ASSERT_NE(encodedGeekCoseKey->asBstr(), nullptr);

    auto geek = CoseKey::parse(encodedGeekCoseKey->asBstr()->value());
    ASSERT_TRUE(geek) << "Error: " << geek.message();

    const std::vector<uint8_t> empty;
    EXPECT_THAT(eekId, ElementsAreArray(geek->getBstrValue(CoseKey::KEY_ID).value_or(empty)));
    EXPECT_THAT(eekPub, ElementsAreArray(geek->getBstrValue(CoseKey::PUBKEY_X).value_or(empty)));
}

TEST(RemoteProvUtilsTest, JsonEncodeCsr) {
    const std::string kSerialNoProp = "ro.serialno";
    cppbor::Array array;
    array.add(1);

    auto [json, error] = jsonEncodeCsrWithBuild(std::string("test"), array, kSerialNoProp);

    ASSERT_TRUE(error.empty()) << error;

    std::string expected = R"({"build_fingerprint":")" +
                           ::android::base::GetProperty("ro.build.fingerprint", /*default=*/"") +
                           R"(","csr":"gQE=","name":"test","serialno":")" +
                           ::android::base::GetProperty("ro.serialno", /*default=*/"") + R"("})";

    ASSERT_EQ(json, expected);
}

TEST(RemoteProvUtilsTest, GenerateEcdsaEekChainInvalidLength) {
    ASSERT_FALSE(generateEekChain(RpcHardwareInfo::CURVE_P256, 1, /*eekId=*/{}));
}

TEST(RemoteProvUtilsTest, GenerateEcdsaEekChain) {
    bytevec kTestEekId = {'t', 'e', 's', 't', 'I', 'd', 0};
    for (size_t length : {2, 3, 31}) {
        auto get_eek_result = generateEekChain(RpcHardwareInfo::CURVE_P256, length, kTestEekId);
        ASSERT_TRUE(get_eek_result) << get_eek_result.message();

        auto& [chain, pubkey, privkey] = *get_eek_result;

        auto validation_result = validateAndExtractEcdsa256EekPubAndId(
            /*testMode=*/true, KeymasterBlob(chain.data(), chain.size()));
        ASSERT_TRUE(validation_result.isOk());

        auto& [eekPubX, eekPubY, eekId] = *validation_result;
        bytevec eekPub;
        eekPub.insert(eekPub.begin(), eekPubX.begin(), eekPubX.end());
        eekPub.insert(eekPub.end(), eekPubY.begin(), eekPubY.end());
        EXPECT_THAT(eekId, ElementsAreArray(kTestEekId));
        EXPECT_THAT(eekPub, ElementsAreArray(pubkey));
    }
}

TEST(RemoteProvUtilsTest, GetProdEcdsaEekChain) {
    auto chain = getProdEekChain(RpcHardwareInfo::CURVE_P256);

    auto validation_result = validateAndExtractEcdsa256EekPubAndId(
        /*testMode=*/false, KeymasterBlob(chain.data(), chain.size()));
    ASSERT_TRUE(validation_result.isOk()) << "Error: " << validation_result.moveError();

    auto& [eekPubX, eekPubY, eekId] = *validation_result;

    auto [geekCert, ignoredNewPos, error] =
        cppbor::parse(kCoseEncodedEcdsa256GeekCert, sizeof(kCoseEncodedEcdsa256GeekCert));
    ASSERT_NE(geekCert, nullptr) << "Error: " << error;
    ASSERT_NE(geekCert->asArray(), nullptr);

    auto& encodedGeekCoseKey = geekCert->asArray()->get(kCoseSign1Payload);
    ASSERT_NE(encodedGeekCoseKey, nullptr);
    ASSERT_NE(encodedGeekCoseKey->asBstr(), nullptr);

    auto geek = CoseKey::parse(encodedGeekCoseKey->asBstr()->value());
    ASSERT_TRUE(geek) << "Error: " << geek.message();

    const std::vector<uint8_t> empty;
    EXPECT_THAT(eekId, ElementsAreArray(geek->getBstrValue(CoseKey::KEY_ID).value_or(empty)));
    EXPECT_THAT(eekPubX, ElementsAreArray(geek->getBstrValue(CoseKey::PUBKEY_X).value_or(empty)));
    EXPECT_THAT(eekPubY, ElementsAreArray(geek->getBstrValue(CoseKey::PUBKEY_Y).value_or(empty)));
}

TEST(RemoteProvUtilsTest, validateBccDegenerate) {
    auto [bcc, _, errMsg] = cppbor::parse(kDegenerateBcc);
    ASSERT_TRUE(bcc) << "Error: " << errMsg;

    EXPECT_TRUE(validateBcc(bcc->asArray(), hwtrust::DiceChain::Kind::kVsr16,
                            /*allowAnyMode=*/false, /*allowDegenerate=*/true));
    EXPECT_FALSE(validateBcc(bcc->asArray(), hwtrust::DiceChain::Kind::kVsr16,
                             /*allowAnyMode=*/false, /*allowDegenerate=*/false));
}
}  // namespace
}  // namespace aidl::android::hardware::security::keymint::remote_prov
