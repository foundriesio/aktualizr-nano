macro( add_libtufnano_client_targets )
        list( APPEND LIBTUFNANO_CLIENT_INCLUDE_DIRS
                ${TUF_SAMPLE_CLIENT_INCLUDE_PUBLIC_DIRS}
        )

        add_library( libtufnano_client STATIC
                ${TUF_SAMPLE_CLIENT_SOURCES}
        )

        target_include_directories( libtufnano_client PUBLIC
                ${TUF_INCLUDE_PUBLIC_DIRS}
                ${TUF_SAMPLE_CLIENT_INCLUDE_PUBLIC_DIRS}
                ${MBEDTLS_INCLUDE_DIRS}
                ${JSON_INCLUDE_PUBLIC_DIRS}
        )
endmacro()

macro( add_libtufnano_targets )
        list( APPEND LIBTUFNANO_CLIENT_INCLUDE_DIRS
                ${TUF_SAMPLE_CLIENT_INCLUDE_PUBLIC_DIRS}
        )

        add_library( libtufnano STATIC
                ${TUF_SOURCES}
                ${TUF_FIAT_SOURCES}
        )

        target_include_directories( libtufnano PUBLIC
                ${TUF_INCLUDE_PUBLIC_DIRS}
                ${TUF_SAMPLE_CLIENT_INCLUDE_PUBLIC_DIRS}
                ${MBEDTLS_INCLUDE_DIRS}
                ${JSON_INCLUDE_PUBLIC_DIRS}
        )
endmacro()
