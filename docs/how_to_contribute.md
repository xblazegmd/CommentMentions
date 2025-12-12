# So you wanna contribute?
Great! This guide's just for you. Since there's many ways you (yes, you) can contribute to this mod, and many other things you need to have in mind when doing so. Anyways, let's get to it!

## First of all...
Make sure you've read [CONTRIBUTING.md](../CONTRIBUTING.md) beforehand, as it contains a bunch of rules you need to have in mind when contributing. If you haven't, well, read it!

## Playtesting
Wanna playtestthe pre-release versions of this mod? Well, here's a quick how-to guide on how to playtest the mod

### Download the pre-release version of the mod
Pre-release versions are available on https://github.com/xblazegmd/CommentMentions/releases. You'll know if the version is a pre-release if:
- It has the `Pre-release` mark (duh)
- It contains pre-release suffixes. Pre-release suffixes contain:
    - -dev
    - -alpha.*
    - -beta.*
    - -rc.*

### Install the mod on Geode
With the .geode file downloaded, open Geode, open the Geode menu in-game, and choose the button with a "file with a plus sign inside" like icon. Now choose the .geode file, and restart GD. The mod should be installed!

### Try it out!
Just use the mod how it's meant to be used. Don't just test it in this lab sorta way where you test the mod to it's limits, no, just try to use it properly. Found an issue? Report it to the GitHub with the guide below. Have suggestions? Post them in the GitHub with the guide below. That is what makes good playtesters.

## Bug reporting
Found a bug in the mod? Well GitHub has this handy Issues tool in which you can report bugs, ask questions, etc. But here's a how-to guide on how to report bugs specifically.

### Make sure the bug is related with this mod specifically
For what we know, the bug could be with this mod, with BetterInfo, with CBF, maybe even Geode itself! Try disabling all of your mods with the exception of CommentMentions. If it dissapears, then it's not related with CommentMentions at all. Try disabling CommentMentions and enabling another mod, then disabling that mod and enabling another, until you found the mod that caused the bug. Then report the bug to their GitHub if they have one (make sure to read their CONTRIBUTING.md)

If the bug still persists, even with only CommentMentions enabled, try disabling CommentMentions as well (or loading Geode in safe mode). If the bug still persists, it's a Geode related bug. Report that in the [Geode issues page](https://github.com/geode-sdk/geode/issues). Make sure to read their CONTRIBUTING.md. If it dissapears, the it is 100% a CommentMentions bug.

### Get more information
Get everything you can that can help with debugging. Whether it's screenshots, game logs, crash logs (if the bug specifically crashes the game), etc, all of them are aprecciated. In the case of game logs, it's reccomended to only show the ones relevant to the bug (like error logs, or just only the logs by the mod itself). It's a waste of time to show logs that have nothing to do with the bug, let alone the mod.

### Create a new issue
Go to the "Issues" tab on the repository, and select "New Issue". Now, select the "Bug Report" form, and fill it in with the information specified in there. Have in mind that if the bug is a crash, you *will* be required to put in the crash logs.

Now, with that done, press "Create", and you just reported the bug!

### Follow up
If I request more information about the bug then you should follow up on the request. At the end of the day, this is a team effort with the sole objective of working together to fix this stupid bug. Nobody likes bugs.

## Suggestions
Suggestions can also be done with GitHub issues. Here's another how-to guide on how to suggest changes.

### Get a solid idea
Think about what you want to do. Is it a feature? A refactor? How could it be impletented? It's reccomended to concretize your request into smaller chunks. For example:

> *"The notifications could be nicer"*

What exactly does "nicer" mean here? Try to break it into smaller problems like:

> *"The notifications could have a nicer UI"*

> *"The notifications could change their UI to be like this*"

And optionally:

> *"The notifications could change their UI to be like this by changing that*"

Now that's a better suggestion, isn't it?

### Make a new issue
Now, go to the "Issues" tab on the repository, and select "New Issue". Select the "Suggestion" template, and fill in the form with the information specified in there. Make sure to add tags related to what specifically your suggestion is (e.g. a refactor, doc changes, etc).

If your suggestion is a **feature** tho, that has it's own separate form. So instead of selecting the "Suggestion" form, select the "Feature Suggestion" form instead. Again, fill it in with the specified information.

Now, with that done, press "Create", and you just made a suggestion!

### Follow up
If I have any issues, suggestions, or just want more information, it's very important for you to follow up to me. How else am I gonna see your vision?

## Adding new features
So you wanna add a new feature? Well, first of all, make sure to read and follow the [code styling guidelines](code_styling.md). They're just there to ensure code styling is consistent across all of the code (and to make sure .NET developers don't ruin my code, you'll see what I mean).

But anyways, let's get to it!

### Some requirements
To work on adding features to this mod (and just developing a Geode mod in general), it's required to have:
- The Geode SDK (duh)
- A device with Geode installed (to test the mod)
- A C++ compiler
- CMake
- Git (ofc)
- The [Geode CLI](https://docs.geode-sdk.org/getting-started/geode-cli)

### Think about your feature
What do you want to add? How do you plan to add it? How will it be compatible with other platforms? All of these questions and more should be answered by you. Think about them. Just like with making suggestions, it's a good principle to concretize your idea.

### Suggest your feature
While not absolutely required, if you're unsure about some areas, want help from others to develop your feature, or just want aproval to make it, it's a good idea to suggest your feature to us (see [suggestions](#suggestions)). If we like it, you can start working on it!

### Fork the repository
In the main GitHub repo page, click on the "Fork" button to make a new fork of the repo. This will be a "copy" of the repo where you can work on your feature.

It's reccomended to make a new branch to work on the feature, instead of working directly on `main`. It's reccomended to name it like the feature. When making commits, make sure to use the [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) scheme. It'll make commits nicer to read.

Now, in your local machine, run the following command:
```sh
git clone https://github.com/<your-github-username>/<your-fork>
cd <your-fork>

# If you made a new branch for working on the feature
git checkout <branch>
```
Now you'll have a clone of your fork in your local machine! In here you can work on your feature in your prefered code editor (VSCode is the prefered editor for developing Geode mods, more information [here](https://docs.geode-sdk.org/getting-started/ide-setup))

### Test your work
Nobody likes an untested feature. Try to test it in as much devices as possible. Try to contact other people to try the mod out in other devices.

### Make a PR
Now, with your feature done, push the changes to your fork.
```sh
git add .
git commit -m "feat: <your-feature-desc>"
git push origin <branch>
```

Now it's time to make a PR! On your fork you should see a "Compare and pull request" button somewhere. Press it to make a new PR. Make sure to target the `dev` branch and not `main`.

Make sure your PR's name follows the [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) scheme. For features it's wise to call it `feat: <your-feature-desc>`.

Make sure to add the "feature" label to your PR

Now click on "Create pull request", and you just made a pull request!

### Follow up
There's an issue with the feature? Follow up on it and fix it. I request a change? Do the change. Now, if the feature has any issues, or I don't like it, or stuff like that happens, the feature *will* be rejected. But if it's good, and I like it, it'll get merged to `dev` and be added in a future release. Congrats, you just added a new feature!

## Adding anything else
Follow all of the steps in the "Adding new features" step, but please have in mind:
- The title of the PR and commit *must* be different than "feat: <desc\>" ofc. Some naming styles (according to the [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) scheme) include:
    - `fix`: Bug fixes
    - `refactor`: Rewriting or restructuring code without altering behavior
    - `perf`: Performance improvements
    - `docs`: Doc changes (doc changes can target `main` instead of `dev`)
    - `build`: Changing build components (e.g. GitHub Actions, CMake, etc)
    - `chore`: Simple tasks (e.g. editing .gitignore)
- As specified, `doc` changes *can* be targeting `main`
- Make sure to add the required labels based on the type of change you're making
- It is still reccomended to make the changes in a separate branch other than `main`

## Anyways...
I hope this guide helped you know how to contribute to the project! Now, read [CONTRIBUTING.md](../CONTRIBUTING.md) if you haven't, and start contributing!