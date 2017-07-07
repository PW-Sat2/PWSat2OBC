def build() {
	bat "cmake -DJLINK_SN=${env.EFM_JLINK_FM} -DMOCK_COM=${env.MOCK_COM_FM} -DOBC_COM=${env.OBC_COM_FM} -DGPIO_COM=${env.GPIO_COM_FM} -DTARGET_MCU_PLATFORM=EngModel -DTARGET_PLD_PLATFORM=Mock -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=1 -G \"MinGW Makefiles\" ../source"
	bat "make pwsat boot fm_terminal"
	step([$class: 'ArtifactArchiver', artifacts: 'build/EngModel/**/*', fingerprint: true])
}

def unitTests() {
	bat "make unit_tests"
	bat "make unit_tests.run -j1"
	step([$class: 'JUnitResultArchiver', testResults: 'build/EngModel/unit_tests_*.xml'])
}

def reports() {
	echo "Memory usage report:"
	bat "make pwsat.memory_report"
	publishHTML(target: [
		allowMissing: false,
		alwaysLinkToLastBuild: false,
		keepAll: false,
		reportDir: 'build/EngModel/reports/memory',
		reportFiles: 'index.html',
		reportName: 'Memory usage'
  ])
}

def integrationTests() {
	lock('hardware') {
		bat "make boot.flash"
		bat "make integration_tests"
		step([$class: 'JUnitResultArchiver', testResults: 'build/EngModel/integration-tests.xml'])
	}
}

def generateDoc() {
	bat "make doc"
	publishHTML(target: [
		allowMissing: false,
		alwaysLinkToLastBuild: false,
		keepAll: false,
		reportDir: 'documentation/html',
		reportFiles: 'index.html',
		reportName: 'Source Code Documentation'
    ])
}

def coverage() {
	bat "cmake -DENABLE_COVERAGE=1 -DCMAKE_BUILD_TYPE=Debug ."
	bat "make unit_tests.coverage"
	publishHTML(target: [
		allowMissing: false,
		alwaysLinkToLastBuild: false,
		keepAll: false,
		reportDir: 'build/EngModel/reports/coverage',
		reportFiles: 'index.html',
		reportName: 'Code Coverage'
    ])
}

node {
	stage 'Checkout'

	try {
		dir('source') {
		  deleteDir()
		  checkout scm
		}

		def toolchainPath = env.ARM_TOOLCHAIN

		withEnv(["PATH+TOOLCHAIN=${toolchainPath}", "PATH+SEGGER=${env.SEGGER}", "CLICOLOR_FORCE=1"]) {
			dir('build') {
				wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'xterm']) {
					deleteDir()

					stage 'Build'
					build()

					stage 'Unit tests'
					unitTests()

					stage 'Reports'
					reports()

					stage concurrency: 1, name: 'Integration Tests'
					integrationTests()

					stage 'Generate Documentation'
					generateDoc()

					stage 'Code Coverage'
					coverage()
				}
			}
		}
	} catch(err) {
		currentBuild.result = 'FAILURE'
	} finally {
		if(currentBuild.result != null) {
			def color = 'danger'

			if(currentBuild.result == 'UNSTABLE')
				color = 'warning'

			slackSend color: color, message: "*Build ${env.JOB_NAME} #${env.BUILD_NUMBER}: ${currentBuild.result}*\n${currentBuild.absoluteUrl}", channel: 'obc-notify'
		}

		step([
			$class: 'WarningsPublisher',
			canResolveRelativePaths: false,
			canRunOnFailed: true,
			consoleParsers:
			[
				[parserName: 'GNU Make + GNU C Compiler (gcc)']
			],
			defaultEncoding: '',
			excludePattern: 'libs/external/**/*.*',
			healthy: '',
			includePattern: '',
			messagesPattern: '',
			unHealthy: '',
			useStableBuildAsReference: true
		])
	}
}
