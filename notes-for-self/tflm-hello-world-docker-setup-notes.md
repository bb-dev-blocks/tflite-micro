# TFLM Hello World Setup Notes (Mac + Docker)

This note explains, in simple terms, what was done to run a basic TensorFlow Lite for Microcontrollers (TFLM) "hello world" example on a Mac (Apple Silicon), what worked, and why it works.

## Goal

Run a small, known-working TFLM example (`hello_world`) in a clean environment using Docker, so the machine does not need manual tool setup.

## What "Hello World" means here

In this repository, `hello_world` is not a text-printing app. It is a tiny machine-learning demo that:

- loads a very small pre-trained model,
- runs inference (prediction),
- checks outputs in a unit test.

The pass condition prints:

- `~~~ALL TESTS PASSED~~~`

If that line appears, the setup and run are successful.

## Why Docker was used

Docker gives a "pre-packed mini Linux machine" with build tools already defined by the repository.

Benefits:

- same environment every run,
- fewer host-machine dependency issues,
- easier to reproduce later.

## Docs used from this repo

Two key docs/files were used:

1. `tensorflow/lite/micro/examples/hello_world/README.md`
   - contains the official command for the hello world test
   - includes the Make command:
     - `make -f tensorflow/lite/micro/tools/make/Makefile test_hello_world_test`

2. `ci/Dockerfile.micro`
   - repository-maintained Docker image for local CI-like debugging
   - includes a documented way to run container with repo mounted

## Step-by-step actions taken

### 1) Confirmed machine and Docker availability

Checked host basics:

- Mac architecture is `arm64` (Apple Silicon).
- Docker is installed and available.

Why this mattered:

- some tooling can be architecture-sensitive,
- we needed to pick a path that works natively on arm64.

### 2) Built the repository Docker image

From repo root:

```bash
docker build -f ci/Dockerfile.micro -t tflm-ci .
```

What this did:

- read the Docker instructions from `ci/Dockerfile.micro`,
- downloaded and installed required packages/tools inside the image,
- created a reusable local image named `tflm-ci`.

What worked:

- image build completed successfully.

### 3) Ran hello_world test inside the container

From repo root:

```bash
docker run --rm -v "$(pwd)":/opt/tflm -w /opt/tflm tflm-ci \
  make -f tensorflow/lite/micro/tools/make/Makefile test_hello_world_test
```

What each part means in plain English:

- `docker run ... tflm-ci`: start a container from image we built.
- `-v "$(pwd)":/opt/tflm`: mount current repo folder into container.
- `-w /opt/tflm`: set working directory inside container.
- `make ... test_hello_world_test`: build and run hello world test target.

What happened during this run:

- it downloaded required third-party code on first run,
- compiled needed binaries/libraries,
- executed the test binary.

Success proof observed:

- output included `~~~ALL TESTS PASSED~~~`.

## Important Apple Silicon note

The chosen command path uses **Make-based** flow, which worked natively on arm64.

Why this was the right choice:

- the hello world README also shows Bazel paths for some tasks,
- but for this setup, the Make test command was the most reliable and direct option for native Apple Silicon in this container flow.

## Convenience improvement added

A helper script was created at repo root:

- `run_hello_world.sh`

What it does:

1. checks whether Docker image `tflm-ci` already exists,
2. builds it if missing,
3. runs the hello world test command in container.

How to use:

```bash
./run_hello_world.sh
```

Why this helps:

- one command instead of remembering long Docker + Make command.

## What worked (summary)

- Docker image build from repository CI Dockerfile: worked.
- Hello world test run in container: worked.
- Test success marker appeared: `~~~ALL TESTS PASSED~~~`.
- Re-runnable helper script added: worked.

## How this works end-to-end (layman view)

Think of it like this:

1. You prepared a special toolbox (`tflm-ci` Docker image).
2. You opened that toolbox and gave it your project files (mounted repo).
3. You asked it to run one specific quality check (`test_hello_world_test`).
4. The check compiled code, ran the tiny ML demo test, and reported pass.

Because Docker packs dependencies and versions, this process is repeatable and less fragile than manual local setup.

## Re-run instructions

Fastest way:

```bash
./run_hello_world.sh
```

Manual way:

```bash
docker build -f ci/Dockerfile.micro -t tflm-ci .
docker run --rm -v "$(pwd)":/opt/tflm -w /opt/tflm tflm-ci \
  make -f tensorflow/lite/micro/tools/make/Makefile test_hello_world_test
```

