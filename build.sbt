
name := "process-jni"

version := "0.2.1"

scalaVersion := "2.11.8"

enablePlugins(JniPlugin, SbtOsgi)

autoScalaLibrary := false

crossPaths := false

parallelExecution in Test := false

// sbt-jni configuration
jniLibraryName := "process"

jniNativeClasses := Seq("com.github.luben.process.Process")

jniLibSuffix := (System.getProperty("os.name").toLowerCase match {
  case os if os startsWith "mac"    => "dylib"
  case os if os startsWith "darwin" => "dylib"
  case _                            => "so"
})

jniNativeCompiler := "gcc"

jniUseCpp11 := false

jniCppExtensions := Seq("c")

jniGccFlags ++= Seq(
  "-Wundef", "-Wshadow", "-Wcast-align", "-Wstrict-prototypes",
  "-Wno-unused-variable"
) ++ (System.getProperty("os.arch") match {
  case "amd64"|"x86_64"   => Seq("-msse4")
  case "i386"             => Seq("-msse4")
  case _                  => Seq()
}) ++ (System.getProperty("os.name").toLowerCase match {
  case "linux" => Seq("-DIS_LINUX=1")
  case _       => Seq()
})

// Where to put the compiled binaries
jniBinPath := {
  val os = System.getProperty("os.name").toLowerCase.replace(' ','_') match {
    case os if os startsWith "mac" => "darwin"
    case os                        => os
  }
  val arch = System.getProperty("os.arch")
  (target in Compile).value / "classes" / os / arch
}

// Where to put the generated headers for the JNI lib
jniHeadersPath := (target in Compile).value / "classes" / "include"

// Sonatype

publishTo := {
  val nexus = "https://oss.sonatype.org/"
  if (version.value.toString.trim.endsWith("SNAPSHOT"))
      Some("snapshots" at nexus + "content/repositories/snapshots")
  else
      Some("releases" at nexus + "service/local/staging/deploy/maven2")
}

publishMavenStyle := true

publishArtifact in Test := false

pomIncludeRepository := { _ => false }

organization := "com.github.luben"

licenses := Seq("BSD 2-Clause License" -> url("https://opensource.org/licenses/BSD-2-Clause"))

description := "JNI bindings for the POSIX/Linux process controll functions"

pomExtra := (
  <url>https://github.com/luben/process-jni</url>
  <scm>
    <url>git@github.com:luben/process-jni.git</url>
    <connection>scm:git:git@github.com:luben/process-jni.git</connection>
  </scm>
  <developers>
    <developer>
      <id>karavelov</id>
      <name>Luben Karavelov</name>
      <email>karavelov@gmail.com</email>
      <organization>com.github.luben</organization>
      <organizationUrl>https://github.com/luben</organizationUrl>
    </developer>
  </developers>
)

// OSGI

osgiSettings

OsgiKeys.bundleSymbolicName := "com.github.luben.process-jni"
OsgiKeys.exportPackage  := Seq(s"""com.github.luben.process;version="${version.value}"""")
OsgiKeys.privatePackage := Seq("com.github.luben.process.util", "include",
  "linux.amd64", "linux.i386", "linux.aarch64", "linux.ppc64",
  "aix.ppc64", "darwin.x86_64"
)
