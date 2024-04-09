FROM ubuntu:jammy as base

# Install dependencies and clean up
RUN apt-get update && apt-get install -y \
    build-essential cmake git wget tmux vim \
    libboost-all-dev && \
    rm -rf /var/lib/apt/lists/*

# set git user name and email
ARG GIT_USERNAME
ARG GIT_USEREMAIL

RUN git config --global user.name ${GIT_USERNAME} && \
    git config --global user.email ${GIT_USEREMAIL} && \
    rm -rf /var/lib/apt/lists/*