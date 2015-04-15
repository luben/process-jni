import sbt._
import Keys._
import com.github.joprice.Jni
import Jni.Keys._
import java.io.File

object ExecBuild extends Build {

  lazy val buildVersion = "0.0.1"

  lazy val root = Project(id="process-jni", base = file("."), settings = Project.defaultSettings).
  settings(
    Jni.settings : _*
  ).settings(
    publishMavenStyle := true,
    version := buildVersion,
    organization := "com.github.luben",
    libraryName := "libprocess",
    gccFlags ++= Seq( "-Wundef", "-Wshadow", "-Wcast-align", "-Wstrict-prototypes", "-Wno-unused-variable"),
    nativeCompiler := "gcc",
    includes += "-I" + nativeSource.value.toString,
    cppExtensions := Seq("c"),
    cpp11 := false,
    jniClasses := Seq("com.github.luben.process.Process"),
    binPath := {
      val os = System.getProperty("os.name").toLowerCase.replace(' ','_')
      val arch =  System.getProperty("os.arch")
      (target in Compile).value / "classes" / os / arch
    },
    headersPath := (target in Compile).value / "classes" / "include",
    publishMavenStyle := true,
    autoScalaLibrary := false,
    crossPaths := false
  )
}
