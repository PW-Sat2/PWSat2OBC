node {
	dir('source') {
	  deleteDir()
	  checkout scm
	}

	def toolchainPath = env.ARM_TOOLCHAIN

	withEnv(["PATH+TOOLCHAIN=${toolchainPath}"]) {
		dir('build') {
		  deleteDir()
		  bat "cmake -G \"MinGW Makefiles\" ../source"
		  bat "make pwsat"
		  bat "make unit_tests.run"
		  echo "Memory usage report:"
		  bat "make pwsat.memory_report"
		}
	}

	step([$class: 'ArtifactArchiver', artifacts: 'build/build/DevBoard/**/*', fingerprint: true])
	step([$class: 'JUnitResultArchiver', testResults: 'build/build/DevBoard/unit-tests.xml'])  
}
