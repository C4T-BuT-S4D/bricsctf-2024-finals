FROM node:lts-buster-slim

RUN apt update && apt install -y curl

ADD package.json /app/package.json
ADD yarn.lock /app/yarn.lock

WORKDIR /app
RUN yarn install

ADD app.env /app/app.env
ADD src /app/src

CMD ["node", "src/main.js"]