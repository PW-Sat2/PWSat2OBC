def build() {
	bat "cmake -DMOCK_COM=${env.MOCK_COM} -DOBC_COM=${env.OBC_COM} -G \"MinGW Makefiles\" ../source"
	bat "make pwsat"
	step([$class: 'ArtifactArchiver', artifacts: 'build/DevBoard/**/*', fingerprint: true])
}

def unitTests() {
	bat "make unit_tests.run"
	step([$class: 'JUnitResultArchiver', testResults: 'build/DevBoard/unit-tests.xml'])
}

def reports() {
	echo "Memory usage report:"
	bat "make pwsat.memory_report"
}

def integrationTests() {
	bat "make integration_tests"
	step([$class: 'JUnitResultArchiver', testResults: 'build/DevBoard/integration-tests.xml'])
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

node {
	stage 'Checkout'

	try {
		dir('source') {
		  deleteDir()
		  checkout scm
		}

		def toolchainPath = env.ARM_TOOLCHAIN

		withEnv(["PATH+TOOLCHAIN=${toolchainPath}", "PATH+SEGGER=${env.SEGGER}"]) {
			dir('build') {
				deleteDir()

				stage 'Build'
				build()

				stage 'Unit tests'
				unitTests()

				stage 'Reports'
				reports()

				stage concurrency: 1, name: 'Integration Tests'
				integrationTests()

				stage name: 'Generate Documentation'
				generateDoc()
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
