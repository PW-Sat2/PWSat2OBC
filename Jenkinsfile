@NonCPS
def cmake(path, generator=null, args) {
	def strings = args.collect { k, v -> "-D${k}=${v}" }
	def g = ""
    if(generator != null) {
        g = "-G \"${generator}\""
    }

    bat("cmake ${g} ${strings.join(" ")} ${path}")
}

def buildPlatform(mcu, pld) {
	stage("Build MCU ${mcu} PLD ${pld}") {
		cmake("../source", "MinGW Makefiles", [
			JLINK_SN: env.EFM_JLINK_FM,
			MOCK_COM: env.MOCK_COM_FM,
			OBC_COM: env.OBC_COM_FM,
			GPIO_COM: env.GPIO_COM_FM,
			TARGET_MCU_PLATFORM: mcu,
			TARGET_PLD_PLATFORM: pld,
			CMAKE_BUILD_TYPE: 'Release',
			ENABLE_LTO: 1,
			COMM_SECURITY_CODE: env.SECURITY_CODE
		])
		bat "make pwsat boot safe_mode pwsat.hex pwsat.bin boot.hex safe_mode.bin generate_telemetry generate_exp_data"
	}
}

def build() {
	buildPlatform('FlightModel', 'FlightModel')
	buildPlatform('EngModel', 'FlightModel')
	buildPlatform('EngModel', 'DM')

	bat("make flatsat_tools")

	step([$class: 'ArtifactArchiver', artifacts: 'build/*/*/bin/*', fingerprint: true])
	step([$class: 'ArtifactArchiver', artifacts: 'build/*/*/tools/*', fingerprint: true])
}

def unitTests() {
	bat "make unit_tests"
	bat "make unit_tests.run -j1"
	step([$class: 'JUnitResultArchiver', testResults: 'build/EngModel/DM/unit_tests_*.xml'])
}

def reports() {
	echo "Memory usage report:"
	bat "make pwsat.memory_report"
	publishHTML(target: [
		allowMissing: false,
		alwaysLinkToLastBuild: false,
		keepAll: false,
		reportDir: 'build/EngModel/DM/reports/memory',
		reportFiles: 'index.html',
		reportName: 'Memory usage'
  ])
  
  bat "make generate_telemetry.run"
  bat "make generate_exp_data.run"
  
  step([$class: 'ArtifactArchiver', artifacts: 'build/*/*/reports/*', fingerprint: true])
}

def integrationTests() {
	lock('hardware') {
		bat "make boot.flash"
		bat "make integration_tests"
		step([$class: 'JUnitResultArchiver', testResults: 'build/EngModel/DM/integration-tests.xml'])
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
	cmake(".", [
		ENABLE_COVERAGE: 1,
		CMAKE_BUILD_TYPE: "Debug"
	])
	bat "make unit_tests.coverage"
	publishHTML(target: [
		allowMissing: false,
		alwaysLinkToLastBuild: false,
		keepAll: false,
		reportDir: 'build/EngModel/DM/reports/coverage',
		reportFiles: 'index.html',
		reportName: 'Code Coverage'
    ])
}

node('pwsat-build') {
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

					build()

					stage 'Unit tests'
					unitTests()

					stage 'Reports'
					reports()

					stage 'Generate Documentation'
					generateDoc()

					stage concurrency: 1, name: 'Integration Tests'
					integrationTests()

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

// if(currentBuild.result == null)
// {
// 	node('flatsat && obc') {
// 		stage('Copy to flatsat') {
// 			dir(env.ARTIFACTS + '\\' + env.BRANCH_NAME) {
// 				unarchive mapping: ['build/': '.']
// 			}
// 		}
// 	}
// }