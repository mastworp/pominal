ARG FEDORA_VERSION=45

FROM fedora:${FEDORA_VERSION} AS toolchain

ARG TOOLS_PYTHON_VERSION=3.14
ARG CLANG_FORMAT_VERSION=22.1.2
ARG CMAKELANG_VERSION=0.6.13
ARG GCOVR_VERSION=8.8

RUN dnf install -y --setopt=install_weak_deps=False \
        cmake \
        gcc-c++ \
        git-core \
        glibc-static \
        libstdc++-static \
        ninja-build \
        python${TOOLS_PYTHON_VERSION} \
        rpm-build \
    && dnf clean all

RUN python${TOOLS_PYTHON_VERSION} -m venv /opt/tools \
    && /opt/tools/bin/pip install --no-cache-dir \
        "clang-format==${CLANG_FORMAT_VERSION}" \
        "cmakelang[YAML]==${CMAKELANG_VERSION}" \
        "gcovr==${GCOVR_VERSION}"

ENV PATH=/opt/tools/bin:${PATH}

WORKDIR /workspace
CMD ["bash"]

FROM toolchain AS build
COPY . .
RUN cmake --workflow --preset package \
    && mkdir -p /dist \
    && cp build/package/*.tar.gz build/package/*.deb build/package/*.rpm /dist/

FROM scratch AS artifacts
COPY --from=build /dist/ /

FROM fedora:${FEDORA_VERSION} AS runtime
RUN --mount=type=bind,from=build,source=/dist,target=/dist \
    dnf install -y /dist/*.rpm \
    && dnf clean all
ENTRYPOINT ["pominal"]
CMD ["--foreground"]
