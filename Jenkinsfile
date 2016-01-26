node {
  echo("Starting build");

  dir('source') {
    checkout scm
  }

  def toolchainPath = env.ARM_TOOLCHAIN

  withEnv(["PATH+TOOLCHAIN=${toolchainPath}"]) {
      dir('build') {
        bat "cmake -G \"MinGW Makefiles\" ../source"
        bat "make pwsat"
        bat "make run_tests"
      }
  }

  step([$class: 'ArtifactArchiver', artifacts: 'build/build/DevBoard/**/*', fingerprint: true])
}
