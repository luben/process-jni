resolvers += Resolver.url("joprice-sbt-plugins", url("https://dl.bintray.com/content/joprice/sbt-plugins"))(Resolver.ivyStylePatterns)
addSbtPlugin("com.github.joprice" % "sbt-jni" % "0.2.1")
addSbtPlugin("com.typesafe.sbt" % "sbt-osgi" % "0.9.4")
