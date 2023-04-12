#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <time.h>

#include "unity.h"

#include "aknano.h"
#include "aknano_debug.h"
#include "aknano_public_api.h"
#include "aknano_net.h"
#include "aknano_targets_manifest.h"
#include "libtufnano.h"

#include "mock_aknano_client.h"
#include "mock_aknano_net.h"
#include "mock_aknano_flash_storage.h"
#include "mock_aknano_board.h"

#include "test_file_signed_fw_4000.c"

#define TEST_BOARD_NAME "MIMXRT1060-EVK"

static const char *json_root_12 = \
"{\"signatures\":[{\"keyid\":\"5c60961d0d950c24276ab23ed74633b75b23cf8678e31842cc0ffcfc8017145d\",\"method\":\"rsassa-pss-sha256\",\"sig\":\"rtQntRMLDzT8RhByOJ5vGBtv/UxMgAFGiKEw3ojeW5zwZwdhA4S53ic6MQynIyr9MOEdXhQ7Cnd6r61B/cxaluBQy8Dg/mGFBt+BDr9M8ZQ8+00p8EaI03grHQ9Q0fT3GstR6PGFA2d6tnkuHarrSqxUXfktOaiGrvNsz3jhlhwZn1Ag/BIlERiiPNw6ZeBKV7xsBqRaqImtBmkSz7X66Qd2gFEHPGlvzOLSoKQFdoY2DkFmHg3wpWNPv5CbYG5xpD87PxmFJFjQO2jwG/c1gVvJqduTgACrvV6ftzfwbv6PbJcWC1NjymU/5OPu+7WnGK5wjFLd7qsP0HOJFpFcYizVipybc2Ofnb2K5P8omeudLoSYqaboDjOoorGc1/ZmwM8ndERfh6FKOojaZkBH5E7oOA3NFyagtHGhGPMbgO4Q8ZWAB7xlE+SLcArk1hAk02JR6PmNgVCvmvFO5TKZ2tx+zidBQfLKPHxtDZl6HzDhhSA6z/vSJezrx6lXeHTSU5eiCAi9jYMobnJig4brLCDs82HEVZh0a03TF01TsQc/8nTbH3ncbVOkesEooryhyaom1ApQn9lePM22J7kUZ8n/uVK6mr3AVhEMCuZjGsWYIwJC4uJH8GZvZ2+caHoMcufaTiC/cssFVPps+FVZGFl9L/baVsJmXiW/upkc7fY=\"},{\"keyid\":\"64823f905eec809efd5fbbcb3bb1d7f529840788c54439739dab0c85ea24cd5a\",\"method\":\"ed25519\",\"sig\":\"e+/yfnDeXTZLlWK6FEMZ6tPATAw4GiAzVfk2o/UXCsO6PXAZS3fXTOfTeMdFsolB32aQtZ59Wfhp4GkQPlozCA==\"}],\"signed\":{\"_type\":\"Root\",\"consistent_snapshot\":false,\"expires\":\"2024-02-13T19:57:04Z\",\"keys\":{\"0c804c5e8f127e9e63ad26cd2f8ed1830e0b67f6e27e0c7ded109f31d6d27808\":{\"keytype\":\"RSA\",\"keyval\":{\"public\":\"-----BEGIN PUBLIC KEY-----\\nMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAu0VVuVPEBFzngvfH7EBw\\nuJt/V2p/Ua8fR0fjTIeOl2q8JwtdzOU0Z0ji8lntoqIOyiYAosg4VZ/VY5CFsWVM\\ndZBhjiA2ZX6oNcgbSf0X5baEaHSxMGZg2ggvwdkqRzAqUDEDaAsMmbA2xSDr+z+Y\\nQ/trCeMLqSJLUJ74OkiPp5zU/ibIBYV7/K8SsuaDYXRpjpvkRozcriIF+KxgPCXT\\nWJeKipC+2fFGlBLQOu/H6lNC5BNX6dWJvyHECjancEFbwqtFh3blffYQOw+jfsSm\\nEH4NrVHMHAb07kYUASmE44yY6+KvfwlmPP3ghlJaM7D1Z40quvv2SxsH+6q++Xjl\\nQ9cHd1ViBbMcmc5KgjPeohZmcGfCBR62gNwFr85PxMh+o5hvgUsS1ZiI80oHojoW\\naXDFI0DUvZNUMa+zjyg4vvV1/MKAVaq9+10SFiYH3S2nff/J9AdgKUAw+mhiJYWu\\nemHJOCHsN65Vgm011CxycWtcTDdwnNezbMZv0az04fAiiSThOAQAjHL8D4cAylni\\nKEn+9zW7W3hniQw+ZO5ToWW3V8YDPG26HvYnKEmInF6nwjWTri4kLjilwQl+V1F0\\nesoQTmL4jGAYX6s7YpcDxebvHROhCYcRnvWIgqNfCr/nuBPgtgyqMFM0B/juty7n\\n4VO3IogKPYBNeT56QNM+5KMCAwEAAQ==\\n-----END PUBLIC KEY-----\\n\"}},\"5c60961d0d950c24276ab23ed74633b75b23cf8678e31842cc0ffcfc8017145d\":{\"keytype\":\"RSA\",\"keyval\":{\"public\":\"-----BEGIN PUBLIC KEY-----\\nMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAxzMBLTvAB8PUVroDLp8F\\nrHNFKTxshILqaEkstlcSXY2T/aecOgANfWp7Lui5YG8Hrv3DgCVTzOvYSB3HbIf/\\njGPVzGowRtlSnFJEF+Qx70InJbvA1hyBEvCgzE+OgHwNrno1SSNYGKKOqluE8gn/\\nphq0r7PT80pv1RREmhQsqrOZlMTK4X4tAARoM6XuQx7WAZ6J5NBAd63/gA9Le4ZX\\nvw9DSmTwZRIlA0x/3ZaMClSz++1tHuoGqG2+6Up7+FdRgcuQR9fSIZE/sXZ9NA6q\\nReJ/+dGsATotaz7xyl3giGkydR1KoEQKJdFdOv+P7J8gPnC2VqqVDfkl5d6ToNzC\\nzHJOQ/eHN4faPCPQdNcj6+79g/A5omJukaHfRC5sQ27SivJqhAXBBxWflAl8pSFL\\nkE/ku0sP9gyILIdZaYc0UYAEUCojicXeGawDINcigqIMlNu47r4WIzfL7/IxzqwS\\nYlgdjFLobUWO1w94hsICe+cfy+4YQcglBBc60FVGX9muILrW/httQrODuqSbEQDX\\nzM9s4Tw0j5Jo/y/xRMl0HwUgw7tLXgeljBctr207s1oDHPLyOve/tGH+NScgTti5\\nzbGxtu1poipLgPiEiZs/rhYc2f4NxJ8t7zb5Q7UfNtAPyE0gfKtQhvodapkCkY7l\\nQTjb+FkEFMYmUbfrUdwb8nkCAwEAAQ==\\n-----END PUBLIC KEY-----\\n\"}},\"93fd00d2b497ebc81fe8de1fa78c5f621b01c1b45dac1c4138b4b9824ac4b418\":{\"keytype\":\"RSA\",\"keyval\":{\"public\":\"-----BEGIN PUBLIC KEY-----\\nMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA8o7tbIgCHqj4O2m6X07Z\\nVVimsiICloT+6rFAid2hd+XjgsOb342DH0VGURzf3bRhjuWKFaKWnBDjvbsbhuim\\ni3WF9/7XksUzZypWMQDd9/sZv75gkGD7/HGx1Bsi0Nav6sqKxpvq1kMEeUH3KZDF\\nAVFIh+Ih/RtVSc1MF3q2xYCi56l43vq2GD3PdDK7wA6qJQA8+j9ya9YK0mIsuQZu\\nW+PYyzmQg5O73Vdj/jZRmBRtcBIoCHsmpcJJXQbFEysm653t/MTwZNSS5pB9V3WQ\\nEsiU0TPrg3oJ+X3yEQyF+3Tqz0weypg/0PaUTedk/9ssiPZWiMlDWLvvny3ENvbs\\nboMR1YSdiFNHvLROEcantmuEX173n66cSlJut1L86J4a2EaXHAdp8DcGUSyqP7Io\\nDln+CTEv3M8SMffV7faik3rIv898mRgi+fAXPBy/02Znj/ccCuSX5w6w8LNL+nXw\\nO9apNdcostxDCRf9IthpJMH0paEefSYuir27cCbUuM/wnTR1QAVyBMpZlTeGi2m2\\nfjfysPoYbMdt3s5fuuRjOeeEslM9O/lb7W71ekBcC4vhj8/OpLJflIMOrCdx8KyS\\nWcOno6QjkqSoUPcs1qb1eHjUDuWYNlzKuPRV5S5ZBLDU1zTx7EW90rY9Hxc9yjcw\\ndE2HnfcarvvzoOdfEZYFctMCAwEAAQ==\\n-----END PUBLIC KEY-----\\n\"}},\"993ceb25d914bfdd16ed633d7a0a0911fa569295cd960ce7232dadc8ead2ce9a\":{\"keytype\":\"RSA\",\"keyval\":{\"public\":\"-----BEGIN PUBLIC KEY-----\\nMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA0n4eTiMByWc5Dc+itvss\\nBTkOisXajnmxsC66+7SN+v9IVU58XdcJG+Ppj7xy1kzu6encm/Xs/yp0kd63Qsa+\\niv/ojcmQX4NSVoUT4XATFjzdJgADol2nz4ZEcCguqqmUH+rKYBzP2yIysys/DQyw\\n6Xpl5OkHKZviwoHR2iT4jnkpZw/p3EDdu+kHR4bm1OcD75xANgUgV635W7XQ0W2R\\nT+8yWObzML31FbJ7Zkgtcu6lAkEx5A5y3OvFAsV7RoHNo3B5ui/RPVEaBpCElNDg\\nzuGmLvw5t8r+O1i3J098EalSxkBEl4f0tXUdUy/oJWBZT03ffuXWhcdENB+VQlfc\\nwojeKcvIKza2qGkBUpE2RZzKfLsnma78C8xWz2JwaXNaaR888XTZDxRJuaYD2xT2\\nHOyq2EEMSLvOCJV4yWKJlhZk9G4AuCOejOJz0rWTaIvHAQ2XHkZh1WMQr1VrJpAM\\ntevxbh5iNjzJGfl/mbybwdKQ/7TFn1yJBaQnGrVeZao1bQHarU5XhL8+0+VjuQDb\\nDOaUNdYnXhwoOXZMY6VQF6jNBfhVsOGpaeta5sY+o1BXpCkImy5isyPWoJaDI72k\\nH1mU/Gf8RUsre1NWtfUpN+Z7qUJaHoWz8r4xlpdfB6pQUd1Pf+N9RT6uszs8cOdh\\nES0NwTRJtGgrTE3cT1teLosCAwEAAQ==\\n-----END PUBLIC KEY-----\\n\"}},\"d3720b30cf3cbf28895dc352d1c7b78a52da92b5ccc30b3e0a623432c8798c05\":{\"keytype\":\"RSA\",\"keyval\":{\"public\":\"-----BEGIN PUBLIC KEY-----\\nMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAr9zGOcC1JJPLZ8ivIGgX\\nHwARl794ksr+2cdsBp6j8J2Y0amxZ5CJCnivfUTy8iK+H/Gl/koAAgOIm7zVZmRj\\nqs9X4sSjW4K7hC0kwFfvIdSebufGu6LR1ZhSIsMpHuEbzxLpy4HMEXNb/h/Z1SUj\\ntbHhW0HDkCY3J+PAhS4tt94xPEcaCDGponTQQ8n0Sq6LQ6DZp43qI9IfERqh8FOW\\nO0O0hQtq5RbJhWvv+UwsoxD6oN7SGROcUIcX8dlciV+auUkCfzvsCuIONyy3njAh\\nlG3Htnymiv/tKQ6wDEqiBIhIdnOuSEgHAuXzoX70EpDNiVYA3S3pjUbcZykhxWAn\\nKMIZcBxHz/4wCKu9NAIoXT+bOMLaLW0ztDB64HFi5st/e9fujt+8vH2f6lJig/py\\ny5/tV2V//K04wWnA/2DGHy3oQhHuAEb3Ni731X/rpZbzIOau2DbTHaXlCDfNIFq7\\n2Lbg8S+t0z7gAby+81IANKonW4Zwul73lrb/hLFgFK28BBOqGGT5C/+JqZAuGTuK\\n+9gSL4WOsTcbW3VIuIF+FlYy2eeojrymMIE/4N2sZ7kWgCB+Byp29TIfCVkiqgiT\\nFYCYL9y5cHQeTGm0twj6m0zB9whTHeYX5hvNdfu8VrlVO1sHfC5BINaDh1Gcc3Oq\\n87gr7gzkJiMMQgfBov8xH10CAwEAAQ==\\n-----END PUBLIC KEY-----\\n\"}}},\"roles\":{\"root\":{\"keyids\":[\"5c60961d0d950c24276ab23ed74633b75b23cf8678e31842cc0ffcfc8017145d\"],\"threshold\":1},\"snapshot\":{\"keyids\":[\"0c804c5e8f127e9e63ad26cd2f8ed1830e0b67f6e27e0c7ded109f31d6d27808\"],\"threshold\":1},\"targets\":{\"keyids\":[\"d3720b30cf3cbf28895dc352d1c7b78a52da92b5ccc30b3e0a623432c8798c05\",\"93fd00d2b497ebc81fe8de1fa78c5f621b01c1b45dac1c4138b4b9824ac4b418\"],\"threshold\":1},\"timestamp\":{\"keyids\":[\"993ceb25d914bfdd16ed633d7a0a0911fa569295cd960ce7232dadc8ead2ce9a\"],\"threshold\":1}},\"version\":12,\"x-changelog\":{\"message\":\"Rotate all TUF root signing keys\",\"polis-id\":\"61263c864de34394ed8b66e4\",\"timestamp\":\"2023-02-13T19:56:57Z\"}}}";

static const char *json_root_13 = \
"{\"signatures\":[{\"keyid\":\"4643fe525098efa8df0625bf61b348573248f83e7d8871bdd5d3a2296301cf05\",\"method\":\"ed25519\",\"sig\":\"Qj/SOhggNf6/EaCoKzi2v9hM9NdALeaKB9Eg60zWLBQ6MHhQ3fTB7zRkYSSF6Cp/g+dupSoLaf/neSfViTTUBw==\"},{\"keyid\":\"5c60961d0d950c24276ab23ed74633b75b23cf8678e31842cc0ffcfc8017145d\",\"method\":\"rsassa-pss-sha256\",\"sig\":\"Wvnh/VR5h6d5KpD+z0XDTMNnbStwMf91/BTZwmoLBi8gqhFHQVarVA+ZmXHFWrMQ3EDpGb4Zq6QOdv34jUyBkAuxIL+0QBSw32+iW+4av5MGx8Cs8h79VRBlcJ9QXNrA8qln2bF1HstTBNnpsxTzqQ9WLV/UAJEiwaRRl/dERYoPENetknoLzkz4GFR4QWXQiESjAmnRasudbtNocizvpVjco8NfxNZrrG34Q6kT43mr63hBBCQFWtmtha4y4IbN09W6QYEGDhG8pj8/lRluwa34PzBFaGUBaZJpOk+BOmsPJRgbFNbvldtSsPEtyWaV3XtD5NdTF+s8UUS4CvvSPcqX8L9mpsMe4cXQqhxNrvOw5bcCGIlAX7FzYcDLY5qswHI/oHu/OaLad4pmhFOaaT3uRYROo0pgshIzFAe/TvP4/BlPh2zNYGwCKorzD+9tGJX2j1nPlQtaDtPNVVqMKJVP+hjXmsvap1HIF343TdbuE6MRPPPHiSCt+3VqbT9vfnPqaIWbZbUn2DH9UgkQaNGJtdxIIw+tXhAawnugM7432WO5GuaxqAhxJrksIE4bSO++ZZpIEkvaOsUJZEEdBbCIHK/sKvdo55YBBrQzL6KzZJJP0+Qs8C4b81u//LXYD6MGkgQrnBtE9mOzwiwe3MKOqVjaeAec2pffAA5fuL4=\"}],\"signed\":{\"_type\":\"Root\",\"consistent_snapshot\":false,\"expires\":\"2024-03-29T19:24:08Z\",\"keys\":{\"061277e11d1c8b1c6abe97a774bb6f94295e6e7fa6242d8e86356f0ee8ea3cab\":{\"keytype\":\"ED25519\",\"keyval\":{\"public\":\"9b40dd0765651e751355a72aa73d484c3a4415a1c0f644ab4d7005f45ce3d8b8\"}},\"4643fe525098efa8df0625bf61b348573248f83e7d8871bdd5d3a2296301cf05\":{\"keytype\":\"ED25519\",\"keyval\":{\"public\":\"068dd7ee84d376013db9e6ed26d82ab8dd354cb535e70027db45fb60cd491989\"}},\"5ea2e62cebe20bca3b9cf381a87495a4a4350bb7e05bb40620fb79ff17741910\":{\"keytype\":\"ED25519\",\"keyval\":{\"public\":\"6f7624f2e8082c05717745b3ceee2f6a05852e9ae343593385e0efcc8c31f7ee\"}},\"9f3295f7859ecc152b7fd6b43e835b02f9970406c00b813a110eeb1cb3b1eb2c\":{\"keytype\":\"ED25519\",\"keyval\":{\"public\":\"509dea6535be348d889640f427104abfa9c26d025b28e0c1937393ab09d6f979\"}},\"ec7b1bc9253691b59a972f6b35783748366c4fb396832eb7748c87ac4c24e929\":{\"keytype\":\"ED25519\",\"keyval\":{\"public\":\"150c5d3cf1a66e46c9ad79f0d4fbe42619ff4cec55e5b030cd44f2d580bd16d5\"}}},\"roles\":{\"root\":{\"keyids\":[\"4643fe525098efa8df0625bf61b348573248f83e7d8871bdd5d3a2296301cf05\"],\"threshold\":1},\"snapshot\":{\"keyids\":[\"5ea2e62cebe20bca3b9cf381a87495a4a4350bb7e05bb40620fb79ff17741910\"],\"threshold\":1},\"targets\":{\"keyids\":[\"061277e11d1c8b1c6abe97a774bb6f94295e6e7fa6242d8e86356f0ee8ea3cab\",\"ec7b1bc9253691b59a972f6b35783748366c4fb396832eb7748c87ac4c24e929\"],\"threshold\":1},\"timestamp\":{\"keyids\":[\"9f3295f7859ecc152b7fd6b43e835b02f9970406c00b813a110eeb1cb3b1eb2c\"],\"threshold\":1}},\"version\":13,\"x-changelog\":{\"message\":\"Rotate all TUF root signing keys\",\"polis-id\":\"61263c864de34394ed8b66e4\",\"timestamp\":\"2023-03-29T19:24:07Z\"}}}";

static const char *json_timestamp = \
"{\"signatures\":[{\"keyid\":\"9f3295f7859ecc152b7fd6b43e835b02f9970406c00b813a110eeb1cb3b1eb2c\",\"method\":\"ed25519\",\"sig\":\"PjqmXNT3WMAidWkodHSJ7ENEo/ZQ9xl47uTUWEJon25wzUfd0L6zWgadT9FYqppahWKdcnJlAWRaNU9A6hDSAg==\"}],\"signed\":{\"_type\":\"Timestamp\",\"expires\":\"2023-04-28T19:15:59Z\",\"meta\":{\"snapshot.json\":{\"hashes\":{\"sha256\":\"68326c5eb0065a1a1a31fd62edeaefff24f91b557d9efcac4e020566a7216831\"},\"length\":427,\"version\":168}},\"version\":168}}";

static const char *json_snapshot = \
"{\"signatures\":[{\"keyid\":\"5ea2e62cebe20bca3b9cf381a87495a4a4350bb7e05bb40620fb79ff17741910\",\"method\":\"ed25519\",\"sig\":\"pdtnOZwnEHs54okwfO438SfuYv2iZJXvWXID3IBfXVwsB9CyD6kdgyms5jqUJ9cSUhvskaI5Jgo+GIu4Ml7eDw==\"}],\"signed\":{\"_type\":\"Snapshot\",\"expires\":\"2023-04-28T19:15:59Z\",\"meta\":{\"targets.json\":{\"hashes\":{\"sha256\":\"2da7da5c458583890a90ea3b29ae177b703f8be91c5c41eac3856b0c540cb54a\"},\"length\":1033,\"version\":168}},\"version\":168}}";

static const char *json_targets = \
"{\"signatures\":[{\"keyid\":\"061277e11d1c8b1c6abe97a774bb6f94295e6e7fa6242d8e86356f0ee8ea3cab\",\"method\":\"ed25519\",\"sig\":\"A0YRwaf2jrcMnee1Ao2d5//Fp/S3B/iio1Ml9muFdNcnzhWG4V3yo6PThRv4UaVOdesrnYubwt4/IP5QjUTTDg==\"}],\"signed\":{\"_type\":\"Targets\",\"expires\":\"2023-04-28T19:15:59Z\",\"targets\":{\"MIMXRT1060-EVK-v1001\":{\"custom\":{\"createdAt\":\"2023-03-29T19:15:59Z\",\"createdBy\":\"61263c864de34394ed8b66e4\",\"hardwareIds\":[\"MIMXRT1060-EVK\"],\"name\":\"MIMXRT1060-EVK-v1001\",\"tags\":[\"devel\"],\"targetFormat\":\"BINARY\",\"updatedAt\":\"2023-03-29T19:15:59Z\",\"version\":\"1001\"},\"hashes\":{\"sha256\":\"3566613982b3935eaad38f5468abf916d17b24409c2f5793b8fe1ec4b0e12f0a\"},\"length\":52560},\"MIMXRT1060-EVK-v4000\":{\"custom\":{\"createdAt\":\"2023-03-29T19:12:03Z\",\"createdBy\":\"61263c864de34394ed8b66e4\",\"hardwareIds\":[\"MIMXRT1060-EVK\"],\"name\":\"MIMXRT1060-EVK-v4000\",\"tags\":[\"devel\"],\"targetFormat\":\"BINARY\",\"updatedAt\":\"2023-03-29T19:12:03Z\",\"version\":\"4000\"},\"hashes\":{\"sha256\":\"6412338049eed6bf15a9a8a5fe8b1b2773a9a4306a7610af30529ea16157a8a8\"},\"length\":52560}},\"version\":168}}";
#define JSON_TARGETS_HIGHER_VERSION 4000

#define TEST_TAG "devel"
static const char *config_toml = \
"{\"z-50-fioctl.toml\":{\"Value\":\"[pacman]\\ntags = \\\"" TEST_TAG "\\\"\\n\",\"Unencrypted\":true,\"OnChanged\":[\"/usr/share/fioconfig/handlers/aktualizr-toml-update\"]}}";

#define JSON_INVALID "{"
#define JSON_EMPTY "{}"

#define JSON_SIMPLE "{\"signed\":{\"targets\":{}}}"

#define JSON_VALID_ONE_TARGET    "{\"signatures\":[{\"keyid\":\"d3720b30cf3cbf28895dc352d1c7b78a52da92b5ccc30b3e0a623432c8798c05\",\"method\":\"rsassa-pss-sha256\",\"sig\":\"rZk5GQqXrYZTc3DV07uMvgG1a/k3PCbWvrE6MtV7C6l/YcpUKbgiLDeOdgEmG6+HAz4u9CoDuFrQCLOwkLk5dWbrshQyS3jMcDOu+xg4rWkYcCXePaMUsH2HE42rY5RDmuB6oUc/kK1breugAzet/9XaKZWiOhmP/RBajRPMXza1ym+irt0SsoCw5BEzZjmIWqNIa36gDH3AhzK4rUPzuICb0/CbNMxRRX104wyU5sYPC1UuoOYuumgCvw8O9HFwtqk9OUMAYw08+MY1LqAFNtG6btLo4JI/CSldBQAFLuzwqM96y+P1wWxT9aqPwDPyvSpX10fsIXz1YzeWV6Pe6ZUMqmzs+wXCGvwx5s5LSLt4l5rNXLQrQrlK4eS4BxehPTbHxyqYDXzSdDmbaFEOZ61Bwv5LV31nunuue1f91E1jMbRkurgAcs+pq/HlW6dDChKUEXK3cAJiTuU2cGhzD/D8SAXJG+hGqOVH3rgvX4zRsg4lPBBW0ZU3dMD7SD0HpVc2rPqdZ//0PkA4LQmT/bZ0TXASWyqXz/x0FGp5x+m8AedOhx3zjkQnQpdT5FzF8ET+33TJeQyr5fsBHvISI8hjNPETAyvlomPKZa0tZGb/aGS3/gK9tO+nHsvg5QE0CaNuG43SOCkOHsRIJAqVdfya53WdQkrINAhYpqDxPRE=\"}],\"signed\":{\"_type\":\"Targets\",\"expires\":\"2023-04-21T12:32:51Z\",\"targets\":{\"MIMXRT1060-EVK-v4736\":{\"hashes\":{\"sha256\":\"b65b2f1dfa19bbcecf313ad8b149745642edf4c503614425e14930242434a4ba\"},\"length\":642264,\"custom\":{\"createdAt\":\"2023-03-17T14:15:36Z\",\"createdBy\":\"61263c864de34394ed8b66e4\",\"hardwareIds\":[\"MIMXRT1060-EVK\"],\"name\":\"MIMXRT1060-EVK-v4736\",\"tags\":[\"devel\"],\"targetFormat\":\"BINARY\",\"updatedAt\":\"2023-03-17T14:15:36Z\",\"version\":\"4736\"}}},\"version\":165}}"

#define JSON_VALID_TWO_TARGETS    "{\"signatures\":[{\"keyid\":\"d3720b30cf3cbf28895dc352d1c7b78a52da92b5ccc30b3e0a623432c8798c05\",\"method\":\"rsassa-pss-sha256\",\"sig\":\"rZk5GQqXrYZTc3DV07uMvgG1a/k3PCbWvrE6MtV7C6l/YcpUKbgiLDeOdgEmG6+HAz4u9CoDuFrQCLOwkLk5dWbrshQyS3jMcDOu+xg4rWkYcCXePaMUsH2HE42rY5RDmuB6oUc/kK1breugAzet/9XaKZWiOhmP/RBajRPMXza1ym+irt0SsoCw5BEzZjmIWqNIa36gDH3AhzK4rUPzuICb0/CbNMxRRX104wyU5sYPC1UuoOYuumgCvw8O9HFwtqk9OUMAYw08+MY1LqAFNtG6btLo4JI/CSldBQAFLuzwqM96y+P1wWxT9aqPwDPyvSpX10fsIXz1YzeWV6Pe6ZUMqmzs+wXCGvwx5s5LSLt4l5rNXLQrQrlK4eS4BxehPTbHxyqYDXzSdDmbaFEOZ61Bwv5LV31nunuue1f91E1jMbRkurgAcs+pq/HlW6dDChKUEXK3cAJiTuU2cGhzD/D8SAXJG+hGqOVH3rgvX4zRsg4lPBBW0ZU3dMD7SD0HpVc2rPqdZ//0PkA4LQmT/bZ0TXASWyqXz/x0FGp5x+m8AedOhx3zjkQnQpdT5FzF8ET+33TJeQyr5fsBHvISI8hjNPETAyvlomPKZa0tZGb/aGS3/gK9tO+nHsvg5QE0CaNuG43SOCkOHsRIJAqVdfya53WdQkrINAhYpqDxPRE=\"}],\"signed\":{\"_type\":\"Targets\",\"expires\":\"2023-04-21T12:32:51Z\",\"targets\":{\"MIMXRT1060-EVK-v4736\":{\"hashes\":{\"sha256\":\"b65b2f1dfa19bbcecf313ad8b149745642edf4c503614425e14930242434a4ba\"},\"length\":642264,\"custom\":{\"createdAt\":\"2023-03-17T14:15:36Z\",\"createdBy\":\"61263c864de34394ed8b66e4\",\"hardwareIds\":[\"MIMXRT1060-EVK\"],\"name\":\"MIMXRT1060-EVK-v4736\",\"tags\":[\"devel\"],\"targetFormat\":\"BINARY\",\"updatedAt\":\"2023-03-17T14:15:36Z\",\"version\":\"4736\"}},\"MIMXRT1170-EVK-v4541\":{\"hashes\":{\"sha256\":\"eddc6a9428696408856c69a39f4d1db5dc5cd919f224a2eff3df0f3946a909e7\"},\"length\":489588,\"custom\":{\"createdAt\":\"2023-02-13T17:17:46Z\",\"createdBy\":\"61263c864de34394ed8b66e4\",\"hardwareIds\":[\"MIMXRT1170-EVK\"],\"name\":\"MIMXRT1170-EVK-v4541\",\"tags\":[\"devel\"],\"targetFormat\":\"BINARY\",\"updatedAt\":\"2023-02-13T17:17:46Z\",\"version\":\"4541\"}}},\"version\":165}}"


#define JSON_VALID_TWO_TARGETS    "{\"signatures\":[{\"keyid\":\"d3720b30cf3cbf28895dc352d1c7b78a52da92b5ccc30b3e0a623432c8798c05\",\"method\":\"rsassa-pss-sha256\",\"sig\":\"rZk5GQqXrYZTc3DV07uMvgG1a/k3PCbWvrE6MtV7C6l/YcpUKbgiLDeOdgEmG6+HAz4u9CoDuFrQCLOwkLk5dWbrshQyS3jMcDOu+xg4rWkYcCXePaMUsH2HE42rY5RDmuB6oUc/kK1breugAzet/9XaKZWiOhmP/RBajRPMXza1ym+irt0SsoCw5BEzZjmIWqNIa36gDH3AhzK4rUPzuICb0/CbNMxRRX104wyU5sYPC1UuoOYuumgCvw8O9HFwtqk9OUMAYw08+MY1LqAFNtG6btLo4JI/CSldBQAFLuzwqM96y+P1wWxT9aqPwDPyvSpX10fsIXz1YzeWV6Pe6ZUMqmzs+wXCGvwx5s5LSLt4l5rNXLQrQrlK4eS4BxehPTbHxyqYDXzSdDmbaFEOZ61Bwv5LV31nunuue1f91E1jMbRkurgAcs+pq/HlW6dDChKUEXK3cAJiTuU2cGhzD/D8SAXJG+hGqOVH3rgvX4zRsg4lPBBW0ZU3dMD7SD0HpVc2rPqdZ//0PkA4LQmT/bZ0TXASWyqXz/x0FGp5x+m8AedOhx3zjkQnQpdT5FzF8ET+33TJeQyr5fsBHvISI8hjNPETAyvlomPKZa0tZGb/aGS3/gK9tO+nHsvg5QE0CaNuG43SOCkOHsRIJAqVdfya53WdQkrINAhYpqDxPRE=\"}],\"signed\":{\"_type\":\"Targets\",\"expires\":\"2023-04-21T12:32:51Z\",\"targets\":{\"MIMXRT1060-EVK-v4736\":{\"hashes\":{\"sha256\":\"b65b2f1dfa19bbcecf313ad8b149745642edf4c503614425e14930242434a4ba\"},\"length\":642264,\"custom\":{\"createdAt\":\"2023-03-17T14:15:36Z\",\"createdBy\":\"61263c864de34394ed8b66e4\",\"hardwareIds\":[\"MIMXRT1060-EVK\"],\"name\":\"MIMXRT1060-EVK-v4736\",\"tags\":[\"devel\"],\"targetFormat\":\"BINARY\",\"updatedAt\":\"2023-03-17T14:15:36Z\",\"version\":\"4736\"}},\"MIMXRT1170-EVK-v4541\":{\"hashes\":{\"sha256\":\"eddc6a9428696408856c69a39f4d1db5dc5cd919f224a2eff3df0f3946a909e7\"},\"length\":489588,\"custom\":{\"createdAt\":\"2023-02-13T17:17:46Z\",\"createdBy\":\"61263c864de34394ed8b66e4\",\"hardwareIds\":[\"MIMXRT1170-EVK\"],\"name\":\"MIMXRT1170-EVK-v4541\",\"tags\":[\"devel\"],\"targetFormat\":\"BINARY\",\"updatedAt\":\"2023-02-13T17:17:46Z\",\"version\":\"4541\"}}},\"version\":165}}"
#define JSON_INVALID_CHAR_IN_HASH "{\"signatures\":[{\"keyid\":\"d3720b30cf3cbf28895dc352d1c7b78a52da92b5ccc30b3e0a623432c8798c05\",\"method\":\"rsassa-pss-sha256\",\"sig\":\"rZk5GQqXrYZTc3DV07uMvgG1a/k3PCbWvrE6MtV7C6l/YcpUKbgiLDeOdgEmG6+HAz4u9CoDuFrQCLOwkLk5dWbrshQyS3jMcDOu+xg4rWkYcCXePaMUsH2HE42rY5RDmuB6oUc/kK1breugAzet/9XaKZWiOhmP/RBajRPMXza1ym+irt0SsoCw5BEzZjmIWqNIa36gDH3AhzK4rUPzuICb0/CbNMxRRX104wyU5sYPC1UuoOYuumgCvw8O9HFwtqk9OUMAYw08+MY1LqAFNtG6btLo4JI/CSldBQAFLuzwqM96y+P1wWxT9aqPwDPyvSpX10fsIXz1YzeWV6Pe6ZUMqmzs+wXCGvwx5s5LSLt4l5rNXLQrQrlK4eS4BxehPTbHxyqYDXzSdDmbaFEOZ61Bwv5LV31nunuue1f91E1jMbRkurgAcs+pq/HlW6dDChKUEXK3cAJiTuU2cGhzD/D8SAXJG+hGqOVH3rgvX4zRsg4lPBBW0ZU3dMD7SD0HpVc2rPqdZ//0PkA4LQmT/bZ0TXASWyqXz/x0FGp5x+m8AedOhx3zjkQnQpdT5FzF8ET+33TJeQyr5fsBHvISI8hjNPETAyvlomPKZa0tZGb/aGS3/gK9tO+nHsvg5QE0CaNuG43SOCkOHsRIJAqVdfya53WdQkrINAhYpqDxPRE=\"}],\"signed\":{\"_type\":\"Targets\",\"expires\":\"2023-04-21T12:32:51Z\",\"targets\":{\"MIMXRT1060-EVK-v4736\":{\"hashes\":{\"sha256\":\"B_5b2f1dfa19bbcecf313ad8b149745642edf4c503614425e14930242434a4ba\"},\"length\":642264,\"custom\":{\"createdAt\":\"2023-03-17T14:15:36Z\",\"createdBy\":\"61263c864de34394ed8b66e4\",\"hardwareIds\":[\"MIMXRT1060-EVK\"],\"name\":\"MIMXRT1060-EVK-v4736\",\"tags\":[\"devel\"],\"targetFormat\":\"BINARY\",\"updatedAt\":\"2023-03-17T14:15:36Z\",\"version\":\"4736\"}},\"MIMXRT1170-EVK-v4541\":{\"hashes\":{\"sha256\":\"eddc6a9428696408856c69a39f4d1db5dc5cd919f224a2eff3df0f3946a909e7\"},\"length\":489588,\"custom\":{\"createdAt\":\"2023-02-13T17:17:46Z\",\"createdBy\":\"61263c864de34394ed8b66e4\",\"hardwareIds\":[\"MIMXRT1170-EVK\"],\"name\":\"MIMXRT1170-EVK-v4541\",\"tags\":[\"devel\"],\"targetFormat\":\"BINARY\",\"updatedAt\":\"2023-02-13T17:17:46Z\",\"version\":\"4541\"}}},\"version\":165}}"

#define JSON_NO_SHA256    "{\"signatures\":[{\"keyid\":\"d3720b30cf3cbf28895dc352d1c7b78a52da92b5ccc30b3e0a623432c8798c05\",\"method\":\"rsassa-pss-sha256\",\"sig\":\"rZk5GQqXrYZTc3DV07uMvgG1a/k3PCbWvrE6MtV7C6l/YcpUKbgiLDeOdgEmG6+HAz4u9CoDuFrQCLOwkLk5dWbrshQyS3jMcDOu+xg4rWkYcCXePaMUsH2HE42rY5RDmuB6oUc/kK1breugAzet/9XaKZWiOhmP/RBajRPMXza1ym+irt0SsoCw5BEzZjmIWqNIa36gDH3AhzK4rUPzuICb0/CbNMxRRX104wyU5sYPC1UuoOYuumgCvw8O9HFwtqk9OUMAYw08+MY1LqAFNtG6btLo4JI/CSldBQAFLuzwqM96y+P1wWxT9aqPwDPyvSpX10fsIXz1YzeWV6Pe6ZUMqmzs+wXCGvwx5s5LSLt4l5rNXLQrQrlK4eS4BxehPTbHxyqYDXzSdDmbaFEOZ61Bwv5LV31nunuue1f91E1jMbRkurgAcs+pq/HlW6dDChKUEXK3cAJiTuU2cGhzD/D8SAXJG+hGqOVH3rgvX4zRsg4lPBBW0ZU3dMD7SD0HpVc2rPqdZ//0PkA4LQmT/bZ0TXASWyqXz/x0FGp5x+m8AedOhx3zjkQnQpdT5FzF8ET+33TJeQyr5fsBHvISI8hjNPETAyvlomPKZa0tZGb/aGS3/gK9tO+nHsvg5QE0CaNuG43SOCkOHsRIJAqVdfya53WdQkrINAhYpqDxPRE=\"}],\"signed\":{\"_type\":\"Targets\",\"expires\":\"2023-04-21T12:32:51Z\",\"targets\":{\"MIMXRT1060-EVK-v4736\":{\"length\":642264,\"custom\":{\"createdAt\":\"2023-03-17T14:15:36Z\",\"createdBy\":\"61263c864de34394ed8b66e4\",\"hardwareIds\":[\"MIMXRT1060-EVK\"],\"name\":\"MIMXRT1060-EVK-v4736\",\"tags\":[\"devel\"],\"targetFormat\":\"BINARY\",\"updatedAt\":\"2023-03-17T14:15:36Z\",\"version\":\"4736\"}}},\"version\":165}}"



// #define JSON_INVALID_TARGET "{\"signed\":{\"targets\":{\"t1\":{\"xxx\": $1}}}}"

void test_aknano_targets_manifest( void )
{
    const char *data = "ERROR";
    int ret;
    struct aknano_context aknano_context = {0};
    struct aknano_settings aknano_settings = {0};

    aknano_delay_Ignore();
    aknano_context.settings = &aknano_settings;

    ret = parse_targets_metadata(JSON_EMPTY, strlen(JSON_EMPTY), NULL);
    TEST_ASSERT_EQUAL(-EINVAL, ret);


    ret = parse_targets_metadata(JSON_EMPTY, strlen(JSON_EMPTY), &aknano_context);
    TEST_ASSERT_EQUAL(-EINVAL, ret);

    aknano_settings.hwid = TEST_BOARD_NAME;
    aknano_settings.tag[0] = 0;
    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_EMPTY, strlen(JSON_EMPTY), &aknano_context);
    TEST_ASSERT_EQUAL(-EINVAL, ret);

    aknano_settings.hwid = NULL;
    strcpy(aknano_settings.tag, "devel");
    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_EMPTY, strlen(JSON_EMPTY), &aknano_context);
    TEST_ASSERT_EQUAL(-EINVAL, ret);

    aknano_settings.hwid = "MIMXRT1060-EVK";
    strcpy(aknano_settings.tag, "devel");
    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_EMPTY, strlen(JSON_EMPTY), &aknano_context);
    TEST_ASSERT_EQUAL(TUF_ERROR_FIELD_MISSING, ret);

    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_INVALID, strlen(JSON_INVALID), &aknano_context);
    TEST_ASSERT_EQUAL(TUF_ERROR_INVALID_METADATA, ret);

    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_INVALID, strlen(JSON_INVALID), &aknano_context);
    TEST_ASSERT_EQUAL(TUF_ERROR_INVALID_METADATA, ret);

    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_SIMPLE, strlen(JSON_SIMPLE), &aknano_context);
    TEST_ASSERT_EQUAL(TUF_SUCCESS, ret);

    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_VALID_ONE_TARGET, strlen(JSON_VALID_ONE_TARGET), &aknano_context);
    TEST_ASSERT_EQUAL(TUF_SUCCESS, ret);

    aknano_settings.hwid = "NON_EXISTING";
    strcpy(aknano_settings.tag, "devel");
    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_VALID_ONE_TARGET, strlen(JSON_VALID_ONE_TARGET), &aknano_context);
    TEST_ASSERT_EQUAL(TUF_SUCCESS, ret);

    aknano_settings.hwid = TEST_BOARD_NAME;
    strcpy(aknano_settings.tag, "devel");
    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_VALID_TWO_TARGETS, strlen(JSON_VALID_TWO_TARGETS), &aknano_context);
    TEST_ASSERT_EQUAL(TUF_SUCCESS, ret);

    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_INVALID_CHAR_IN_HASH, strlen(JSON_INVALID_CHAR_IN_HASH), &aknano_context);
    TEST_ASSERT_EQUAL(TUF_SUCCESS, ret);

    memset(&aknano_context.selected_target, 0, sizeof(aknano_context.selected_target));
    ret = parse_targets_metadata(JSON_NO_SHA256, strlen(JSON_NO_SHA256), &aknano_context);
    TEST_ASSERT_EQUAL(TUF_SUCCESS, ret);
}

/* March 29, 2023 8:05:13 PM */
#define TEST_DEFAULT_INITIAL_EPOCH_MS 1680120313000UL

/* Add 1 month, timestamp.json should be expired */
#define TEST_DEFAULT_INITIAL_EPOCH_MS_EXPIRED TEST_DEFAULT_INITIAL_EPOCH_MS + (1000UL * 60 * 60 * 24 * 30)

static uint64_t test_current_epoch_ms = TEST_DEFAULT_INITIAL_EPOCH_MS;

static void stub_aknano_delay(uint32_t ms, int num_calls)
{
    test_current_epoch_ms += ms;
}

static time_t stub_aknano_cli_get_current_epoch(int num_calls)
{
    return test_current_epoch_ms / 1000;
}

#define TEST_RANDOM_SEED 66736278
static bool test_random_seed_initialized = false;
static status_t stub_aknano_cli_gen_random_bytes(char *output, size_t size, int num_calls)
{
    if (!test_random_seed_initialized) {
        srand(TEST_RANDOM_SEED);
        test_random_seed_initialized = true;
    }

    for (size_t i = 0; i < size; i++) {
        output[i] = rand() & 0xFF;
    }
    return 0;
}

const char *stub_aknano_get_board_name(int num_call)
{
    return TEST_BOARD_NAME;
}

void stub_aknano_get_ipv4_and_mac(uint8_t *ip, uint8_t *mac)
{
    uint8_t default_ip[] = { 192, 0, 0, 10};
    uint8_t default_mac[] = { 0x85, 0x81, 0x82, 0x83, 0x84, 0x85};

    memcpy(ip, default_ip, sizeof(default_ip));
    memcpy(mac, default_mac, sizeof(default_mac));
}

static char in_memory_flash[1024 * 1024 * 8];

status_t stub_aknano_read_flash_storage(int offset, void *output, size_t outputMaxLen, int stub_num_calls)
{
    memcpy(output, in_memory_flash + AKNANO_STORAGE_FLASH_OFFSET + offset, outputMaxLen);

    printf("\r\nstub_num_calls=%d\r\n", stub_num_calls);
    return 0;
}

status_t stub_aknano_write_data_to_flash(int offset, const void *data, size_t data_len, int stub_num_calls)
{
    memcpy(in_memory_flash + offset, data, data_len);
    return 0;
}

status_t stub_aknano_write_data_to_storage(int offset, const void *data, size_t data_len, int stub_num_calls)
{
    return stub_aknano_write_data_to_flash(AKNANO_STORAGE_FLASH_OFFSET + offset, data, data_len, 0);
}

void stub_aknano_update_settings_in_flash(struct aknano_settings *aknano_settings)
{
    char flashPageBuffer[256];
    size_t offset = 0;

    memset(flashPageBuffer, 0, sizeof(flashPageBuffer));


    memcpy(flashPageBuffer + offset, &aknano_settings->last_applied_version, sizeof(int));
    offset += sizeof(int);

    memcpy(flashPageBuffer + offset, &aknano_settings->last_confirmed_version, sizeof(int));
    offset += sizeof(int);

    memcpy(flashPageBuffer + offset, aknano_settings->ongoing_update_correlation_id,
           sizeof(aknano_settings->ongoing_update_correlation_id));
    offset += sizeof(aknano_settings->ongoing_update_correlation_id);

    memcpy(flashPageBuffer + offset, &aknano_settings->rollback_retry_count,
           sizeof(aknano_settings->rollback_retry_count));
    offset += sizeof(aknano_settings->rollback_retry_count);

    memcpy(flashPageBuffer + offset, &aknano_settings->rollback_next_retry_time,
           sizeof(aknano_settings->rollback_next_retry_time));
    offset += sizeof(aknano_settings->rollback_next_retry_time);

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    flashPageBuffer[offset] = aknano_settings->is_device_registered;
    offset += 1;
#endif

    LogInfo(("Writing settings to flash..."));
    stub_aknano_write_data_to_storage(AKNANO_FLASH_OFF_STATE_BASE, flashPageBuffer, sizeof(flashPageBuffer), 0);
}

enum test_type_aknano {
    AKTEST_SUCCESS,
    AKTEST_SUCCESS_ROLLED_BACK, /* TODO */
    AKTEST_ERROR_TUF_METADATA_TOO_BIG,
    AKTEST_ERROR_TUF_METADATA_EXPIRED,
    AKTEST_ERROR_TUF_METADATA_DOWNLOAD_FAILED,
    AKTEST_ERROR_INCOMPLETE_DOWNLOAD,
    AKTEST_ERROR_CORRUPTED_DOWNLOAD,
    AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT,
} test_ongoing_test_type;

BaseType_t stub_aknano_mtls_connect(struct aknano_network_context* network_context, const char* hostname, size_t hostname_len, uint16_t port, const char* server_root_ca, size_t server_root_ca_len, int stub_num_calls)
{
    TEST_ASSERT(network_context != NULL);
    TEST_ASSERT(hostname != NULL);
    TEST_ASSERT(hostname_len >= 0);
    TEST_ASSERT(port > 0);

    TEST_ASSERT(!network_context->is_connected);

    if (test_ongoing_test_type == AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT) {
        /* For testing repeated checking failures, emulate a connection failure on every attempt */
        return 0;
    }

    network_context->is_connected = true;
    return 1;
}

#define AKANANO_TEST_VERSION 1000

BaseType_t stub_aknano_mtls_send_http_request(
                                        struct aknano_network_context *network_context,
                                        const char *hostname,
                                        size_t hostname_len,
                                        const char *pcMethod,
                                        const char *pcPath,
                                        const char *pcBody,
                                        size_t xBodyLen,
                                        unsigned char *buffer,
                                        size_t buffer_len,
                                        const char **header_keys,
                                        const char **header_values,
                                        size_t header_len,
                                        int request_range_start,
                                        int request_range_end,
                                        int stub_num_calls)
{
    printf("pcPath=%s method=%s\n", pcPath, pcMethod);

    TEST_ASSERT(hostname != NULL);
    TEST_ASSERT(hostname_len > 0);
    TEST_ASSERT(pcMethod != NULL);
    TEST_ASSERT(strcmp(pcMethod, HTTP_METHOD_GET) == 0 || strcmp(pcMethod, HTTP_METHOD_PUT) == 0 || strcmp(pcMethod, HTTP_METHOD_POST) == 0);

    TEST_ASSERT(network_context->is_connected);
    network_context->reply_body_len = 0;
    network_context->reply_http_code = 0;

    if (strcmp(pcMethod, HTTP_METHOD_GET) == 0) {
        if (strcmp(pcPath, "/config") == 0) {
            network_context->reply_body = config_toml;
            network_context->reply_body_len = strlen(network_context->reply_body);
        } else if (strcmp(pcPath, "/repo/13.root.json") == 0) {
            network_context->reply_body = json_root_13;
            network_context->reply_body_len = strlen(network_context->reply_body);
        } else if (strcmp(pcPath, "/repo/timestamp.json") == 0) {
            network_context->reply_body = json_timestamp;
            network_context->reply_body_len = strlen(network_context->reply_body);
            if (test_ongoing_test_type == AKTEST_ERROR_TUF_METADATA_DOWNLOAD_FAILED) {
                /* Emulate an error on the network operation */
                return 0;
            }
        } else if (strcmp(pcPath, "/repo/snapshot.json") == 0) {
            network_context->reply_body = json_snapshot;
            network_context->reply_body_len = strlen(network_context->reply_body);
        } else if (strcmp(pcPath, "/repo/targets.json") == 0) {
            network_context->reply_body = json_targets;
            network_context->reply_body_len = strlen(network_context->reply_body);
            if (test_ongoing_test_type == AKTEST_ERROR_TUF_METADATA_TOO_BIG) {
                /* For testing handling of "too big" metadata, just send the test fw binary instead */
                network_context->reply_body = random_fw_4000_signed;
                network_context->reply_body_len = random_fw_4000_signed_len;
            }
        } else if (strcmp(pcPath, "/mcu/files/64/12338049eed6bf15a9a8a5fe8b1b2773a9a4306a7610af30529ea16157a8a8.bin") == 0) {
            TEST_ASSERT(request_range_start >= 0);
            TEST_ASSERT(request_range_end > 0);
            size_t file_len = random_fw_4000_signed_len;
            if (test_ongoing_test_type == AKTEST_ERROR_INCOMPLETE_DOWNLOAD)
                file_len = random_fw_4000_signed_len - 100; /* do not send last 100 bytes*/
            network_context->reply_body = random_fw_4000_signed + request_range_start;
            if (request_range_end < file_len) {
                network_context->reply_body_len = request_range_end - request_range_start + 1;
                network_context->reply_http_code = 206;
            } else {
                network_context->reply_body_len = file_len - request_range_start + 1;
                network_context->reply_http_code = 206;
            }
            /* Shift first block by 1 byte to corrupt final donwloaded image, without needing an extra buffer */
            if (test_ongoing_test_type == AKTEST_ERROR_CORRUPTED_DOWNLOAD && request_range_start == 0)
                network_context->reply_body = random_fw_4000_signed + 1;

        } else {
            network_context->reply_body = "";
        }
        if (!network_context->reply_http_code)
            network_context->reply_http_code = network_context->reply_body_len == 0? 404 : 200;
    } else {
        network_context->reply_body = "";
        network_context->reply_body_len = 0;
        network_context->reply_http_code = 204;
    }

    // printf("network_context->reply_body=%s\r\n", network_context->reply_body);

    return 1;
}

int stub_aknano_init_network_context(struct aknano_network_context *network_context, int num_calls)
{
    memset(network_context, 0, sizeof(*network_context));
    return;
}


void stub_aknano_mtls_disconnect(struct aknano_network_context *network_context, int num_calls)
{
    TEST_ASSERT(network_context != NULL);

    if (test_ongoing_test_type != AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT)
        TEST_ASSERT(network_context->is_connected);
    network_context->is_connected = false;
}


uint32_t stub_aknano_get_target_slot_address(uint8_t current_image_position, int num_calls)
{
    if (current_image_position == 0x01)
        return 0x240000;
    else if (current_image_position == 0x02)
        return 0x40000;
    else
        return 0x240000;
}

void static provision_test_tuf_root()
{
    strcpy(in_memory_flash + AKNANO_STORAGE_FLASH_OFFSET + AKNANO_FLASH_OFF_TUF_ROOT_PROVISIONING, json_root_12);
}

void static initialize_test_flash()
{
    memset(in_memory_flash, 0xFF, sizeof(in_memory_flash));
    provision_test_tuf_root();

    uint32_t version = AKANANO_TEST_VERSION;
    memcpy(in_memory_flash + AKNANO_STORAGE_FLASH_OFFSET + AKNANO_FLASH_OFF_LAST_CONFIRMED_VERSION, &version, sizeof(version));
}


void internal_test_aknano_api()
{
    static struct aknano_settings aknano_settings;
    time_t startup_epoch;
    const time_t max_offline_time_on_temp_image = 180;
    bool any_checkin_ok = false;

    test_current_epoch_ms = TEST_DEFAULT_INITIAL_EPOCH_MS;
    if (test_ongoing_test_type == AKTEST_ERROR_TUF_METADATA_EXPIRED)
        test_current_epoch_ms = TEST_DEFAULT_INITIAL_EPOCH_MS_EXPIRED;

    /* Initialization needs to be called once */
    const int image_position = 1;
    uint32_t version = AKANANO_TEST_VERSION;
    aknano_get_image_position_ExpectAndReturn(image_position);
    aknano_get_current_version_Expect(NULL, image_position);
    aknano_get_current_version_IgnoreArg_running_version();
    aknano_get_current_version_ReturnThruPtr_running_version(&version);
    aknano_read_flash_storage_StubWithCallback(stub_aknano_read_flash_storage);
        if (test_ongoing_test_type == AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT)
        aknano_is_current_image_permanent_IgnoreAndReturn(false);
    else
        aknano_is_current_image_permanent_IgnoreAndReturn(true);
    aknano_get_board_name_StubWithCallback(stub_aknano_get_board_name);
    aknano_delay_StubWithCallback(stub_aknano_delay);
    aknano_cli_gen_random_bytes_StubWithCallback(stub_aknano_cli_gen_random_bytes);
    aknano_cli_get_current_epoch_StubWithCallback(stub_aknano_cli_get_current_epoch);

    startup_epoch = aknano_cli_get_current_epoch();
    // aknano_read_flash_storage()
    aknano_init(&aknano_settings);
    /* Make sure we get the right tag from the config file */
    strcpy(aknano_settings.tag, "invalid");

    /*
     * Tell aktualizr-nano that it is OK to set the current image as permanent
     * aktualizr-nano also will only set the image as permanent after a successful
     * checkin is done at the factory device gateway
     */
    aknano_set_application_self_test_ok(&aknano_settings);
    while (true) {
        static struct aknano_context aknano_context;
        int checkin_result;
        int sleep_time;

        /* Initialize execution context */
        aknano_init_context(&aknano_context, &aknano_settings);

        /*
         * Check-in to device gateway, and select target with the highest version
         * that matches the current tag and hardware type
         */

        // init_network_context_ExpectAnyArgsAndReturn(0);
        // init_network_context_IgnoreAndReturn(0);
        init_network_context_StubWithCallback(stub_aknano_init_network_context);
        aknano_mtls_connect_StubWithCallback(stub_aknano_mtls_connect);
        aknano_mtls_disconnect_StubWithCallback(stub_aknano_mtls_disconnect);
        aknano_mtls_send_http_request_StubWithCallback(stub_aknano_mtls_send_http_request);
        aknano_write_data_to_storage_StubWithCallback(stub_aknano_write_data_to_storage);
        if (test_ongoing_test_type != AKTEST_ERROR_TUF_METADATA_EXPIRED
            && test_ongoing_test_type != AKTEST_ERROR_TUF_METADATA_TOO_BIG
            && test_ongoing_test_type != AKTEST_ERROR_TUF_METADATA_DOWNLOAD_FAILED
            && test_ongoing_test_type != AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT) {
            aknano_set_image_confirmed_ExpectAnyArgs();
        }

        aknano_get_ipv4_and_mac_StubWithCallback(stub_aknano_get_ipv4_and_mac);
        // set_ipv4_and_mac_mock();
        checkin_result = aknano_checkin(&aknano_context);

        LogInfo(("checkin_result=%d", checkin_result));

        if (test_ongoing_test_type == AKTEST_ERROR_TUF_METADATA_EXPIRED) {
            /* Error test: expired tuf metadata */
            TEST_ASSERT(checkin_result == TUF_ERROR_EXPIRED_METADATA);
            return;
        }

        if (test_ongoing_test_type == AKTEST_ERROR_TUF_METADATA_TOO_BIG) {
            /* Error test: expired tuf metadata */
            TEST_ASSERT(checkin_result == TUF_ERROR_DATA_EXCEEDS_BUFFER_SIZE);
            return;
        }

        if (test_ongoing_test_type == AKTEST_ERROR_TUF_METADATA_DOWNLOAD_FAILED) {
            /* Error test: download operation failed */
            TEST_ASSERT(checkin_result == -1);
            return;
        }

        if (test_ongoing_test_type == AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT) {
            TEST_ASSERT(checkin_result == -10);
            aknano_context.settings->is_image_permanent = false;
        } else {
            TEST_ASSERT(checkin_result == 0);
            TEST_ASSERT_EQUAL_STRING(TEST_TAG, aknano_context.settings->tag);
        }

        if (checkin_result == 0) {
            /* Check-in successful. Check selected target */
            bool is_update_required = false;
            bool is_reboot_required = false;

            any_checkin_ok = true;
            if (aknano_has_matching_target(&aknano_context)) {
                uint32_t current = aknano_get_current(&aknano_context);
                TEST_ASSERT(current == AKANANO_TEST_VERSION);

                uint32_t selected = aknano_get_selected_version(&aknano_context);
                TEST_ASSERT(selected == JSON_TARGETS_HIGHER_VERSION);

                bool is_rollback = aknano_is_rollback(&aknano_context);
                TEST_ASSERT(is_rollback == false);

                bool should_retry_rollback = false;

                if (is_rollback)
                    should_retry_rollback = aknano_should_retry_rollback(&aknano_context);

                LogInfo(("* Manifest data parsing result: current version=%u selected version=%u is_rollback=%s should_retry_rollback=%s",
                         current, selected, is_rollback? "YES" : "NO", should_retry_rollback? "YES" : "NO"));

                if (is_rollback && !should_retry_rollback) {
                    LogInfo(("* Selected version was already applied (and failed). Do not retrying it"));
                } else if (current < selected) {
                    LogInfo((ANSI_COLOR_GREEN "* Update required: %u -> %u" ANSI_COLOR_RESET,
                             current, selected));
                    is_update_required = true;
                } else {
                    LogInfo(("* No update required"));
                }
            } else {
                TEST_ASSERT(false);
                LogInfo(("* No matching target found in manifest"));
            }
            TEST_ASSERT(is_update_required);

            /* An update is required */
            if (is_update_required) {
                aknano_get_target_slot_address_StubWithCallback(stub_aknano_get_target_slot_address);
                aknano_write_data_to_flash_StubWithCallback(stub_aknano_write_data_to_flash);
                aknano_update_settings_in_flash_StubWithCallback(stub_aknano_update_settings_in_flash);
                if (test_ongoing_test_type == AKTEST_SUCCESS || test_ongoing_test_type == AKTEST_SUCCESS_ROLLED_BACK) {
                    aknano_verify_image_IgnoreAndReturn(true);
                    aknano_set_image_ready_for_test_IgnoreAndReturn(0);
                }
                is_reboot_required = aknano_install_selected_target(&aknano_context);
                if (test_ongoing_test_type == AKTEST_ERROR_INCOMPLETE_DOWNLOAD || test_ongoing_test_type == AKTEST_ERROR_CORRUPTED_DOWNLOAD) {
                    TEST_ASSERT(is_reboot_required == false);
                    return;
                }
            }

            TEST_ASSERT(is_reboot_required);
            /* An update was performed, reboot board */
            if (is_reboot_required) {
                aknano_reboot_command_Expect();
                aknano_reboot_command();
                break;
            }
        } else {
            LogInfo(("* Check-in failed with error %d", checkin_result));
        }

        /* If the checkin operation fails for too long after an update, the image may be bad */
        if (!any_checkin_ok && aknano_is_temp_image(&aknano_settings) &&
            aknano_cli_get_current_epoch() > startup_epoch + max_offline_time_on_temp_image) {
            LogWarn(("* Check-in failed for too long while running a temporary image. Forcing a reboot to initiate rollback process"));
            aknano_delay(2000);
            aknano_reboot_command_Expect();
            aknano_reboot_command();
            break;
        }

        sleep_time = aknano_get_setting(&aknano_context, "polling_interval");
        sleep_time = limit_sleep_time_range(sleep_time);
        LogInfo(("Sleeping %d seconds. any_checkin_ok=%d temp_image=%d\n\n",
                 sleep_time, any_checkin_ok, aknano_is_temp_image(&aknano_settings)));
        aknano_delay(sleep_time * 1000);
        if (test_ongoing_test_type != AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT)
            break;
    }
}

void test_aknano_api_ok()
{
    test_ongoing_test_type = AKTEST_SUCCESS;
    internal_test_aknano_api();
}

void test_aknano_api_expired()
{
    test_ongoing_test_type = AKTEST_ERROR_TUF_METADATA_EXPIRED;
    internal_test_aknano_api();
}

void test_aknano_tuf_metadata_too_big()
{
    test_ongoing_test_type = AKTEST_ERROR_TUF_METADATA_TOO_BIG;
    internal_test_aknano_api();
}

void test_aknano_tuf_metadata_download_failed()
{
    test_ongoing_test_type = AKTEST_ERROR_TUF_METADATA_DOWNLOAD_FAILED;
    internal_test_aknano_api();
}

void test_aknano_incomplete_firmware_download()
{
    test_ongoing_test_type = AKTEST_ERROR_INCOMPLETE_DOWNLOAD;
    internal_test_aknano_api();
}

void test_aknano_corrupted_firmware_download()
{
    test_ongoing_test_type = AKTEST_ERROR_CORRUPTED_DOWNLOAD;
    internal_test_aknano_api();
}

void test_aknano_error_fail_checkin_until_reboot()
{
    test_ongoing_test_type = AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT;
    internal_test_aknano_api();
}

void internal_test_aknano_default_loop()
{
    uint32_t remaining_iterations = 2;

    /* For initialization */
    const int image_position = 1;
    uint32_t version = AKANANO_TEST_VERSION;
    aknano_get_image_position_ExpectAndReturn(image_position);
    aknano_get_current_version_Expect(NULL, image_position);
    aknano_get_current_version_IgnoreArg_running_version();
    aknano_get_current_version_ReturnThruPtr_running_version(&version);
    aknano_read_flash_storage_StubWithCallback(stub_aknano_read_flash_storage);
    if (test_ongoing_test_type == AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT)
        aknano_is_current_image_permanent_IgnoreAndReturn(false);
    else
        aknano_is_current_image_permanent_IgnoreAndReturn(true);
    aknano_get_board_name_StubWithCallback(stub_aknano_get_board_name);
    aknano_delay_StubWithCallback(stub_aknano_delay);
    aknano_cli_gen_random_bytes_StubWithCallback(stub_aknano_cli_gen_random_bytes);
    aknano_cli_get_current_epoch_StubWithCallback(stub_aknano_cli_get_current_epoch);

    /* Per iteration */
    init_network_context_StubWithCallback(stub_aknano_init_network_context);
    aknano_mtls_connect_StubWithCallback(stub_aknano_mtls_connect);
    aknano_mtls_disconnect_StubWithCallback(stub_aknano_mtls_disconnect);
    aknano_mtls_send_http_request_StubWithCallback(stub_aknano_mtls_send_http_request);
    aknano_write_data_to_storage_StubWithCallback(stub_aknano_write_data_to_storage);
    aknano_get_ipv4_and_mac_StubWithCallback(stub_aknano_get_ipv4_and_mac);

    /* Per update */
    if (test_ongoing_test_type != AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT) {
        aknano_set_image_confirmed_ExpectAnyArgs();
        aknano_get_target_slot_address_StubWithCallback(stub_aknano_get_target_slot_address);
        aknano_write_data_to_flash_StubWithCallback(stub_aknano_write_data_to_flash);
        aknano_update_settings_in_flash_StubWithCallback(stub_aknano_update_settings_in_flash);
        aknano_verify_image_IgnoreAndReturn(true);
        aknano_set_image_ready_for_test_IgnoreAndReturn(0);
    }
    aknano_reboot_command_Expect();

    if (test_ongoing_test_type == AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT) {
        remaining_iterations = 1000000;
    }
    aknano_sample_loop(&remaining_iterations);
}

void test_aknano_default_loop()
{
    internal_test_aknano_default_loop();
}

void test_aknano_default_loop_watchdog()
{
    test_ongoing_test_type = AKTEST_ERROR_FAIL_CHECKIN_UNTIL_REBOOT;
    internal_test_aknano_default_loop();
}

/* Dummy implementation for now. Settings API will be improved */
void UpdateSettingValue(const char* name, int value) {}


/* Called before each test method. */
void setUp()
{
    test_ongoing_test_type = AKTEST_SUCCESS;
    test_current_epoch_ms = TEST_DEFAULT_INITIAL_EPOCH_MS;
    test_random_seed_initialized = false;
    initialize_test_flash();
}

/* Called after each test method. */
void tearDown()
{
}
