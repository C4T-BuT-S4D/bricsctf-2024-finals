# FROM amazoncorretto:21-alpine3.17-jdk as build
FROM adoptopenjdk/openjdk16:debianslim as build
WORKDIR /tmp/gradle/compile
COPY gradle ./gradle
COPY *.gradle ./
COPY gradlew ./
COPY src ./src
RUN ./gradlew --no-daemon build

FROM adoptopenjdk/openjdk16:debianslim-jre
WORKDIR /app
COPY --from=build /tmp/gradle/compile/build/libs/* ./
CMD ["java", "-jar", "/app/authrpc-all.jar"]