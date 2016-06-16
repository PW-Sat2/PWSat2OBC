node {
	try {
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

	} catch(err) {
		currentBuild.result = 'FAILURE'
	} finally {
		if(currentBuild.result != null) {
			def color = 'danger'

			if(currentBuild.result == 'UNSTABLE')
				color = 'warning'

			slackSend color: color, message: "*Build ${env.JOB_NAME} #${env.BUILD_NUMBER}: ${currentBuild.result}*\n${currentBuild.absoluteUrl}", channel: 'obc-notify'
		}
	}
}
