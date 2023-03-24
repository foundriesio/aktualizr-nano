#include <errno.h>
#include <string.h>

#include "unity.h"

#include "aknano.h"
#include "aknano_public_api.h"
#include "aknano_net.h"
#include "aknano_targets_manifest.h"
#include "libtufnano.h"
// #include "catch_assert.h"

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

    aknano_context.settings = &aknano_settings;

    ret = parse_targets_metadata(JSON_EMPTY, strlen(JSON_EMPTY), NULL);
    TEST_ASSERT_EQUAL(-EINVAL, ret);


    ret = parse_targets_metadata(JSON_EMPTY, strlen(JSON_EMPTY), &aknano_context);
    TEST_ASSERT_EQUAL(-EINVAL, ret);

    aknano_settings.hwid = "MIMXRT1060-EVK";
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

    aknano_settings.hwid = "MIMXRT1060-EVK";
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

void test_aknano_net( void )
{
    struct aknano_network_context aknano_network_context;

    init_network_context(&aknano_network_context);
    aknano_network_context.source_path = "test_path";
    aknano_mtls_connect(&aknano_network_context, "hostname", strlen("hostname"), 8080, "", strlen(""));
    aknano_mtls_disconnect(&aknano_network_context);
}


void test_sample_loop()
{
    aknano_sample_loop();
}