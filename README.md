# HexEngine

Ferramenta simples para converter qualquer arquivo (binário ou texto)
para o formato hexadecimal. Foi desenvolvida apenas para fins didáticos.

# Baixar e compilar
* Instale o Qt Creator: https://www.qt.io/download
* Instale o Git: https://git-scm.com/
* Faça um clone do repositório: **git clone https://github.com/lucas-engen/HexEngine.git**
* Crie uma variável de ambiente chamado **PASTA_GIT_BIN** e armazene nela o local da pasta bin do git. Exemplo: "C:\Program Files\Git\bin"
* Abra o arquivo hexengine.pro, localizado na mesma pasta que os arquivos-fonte.
* Com o Qt aberto vá em *Projects->Build Steps->Add Build Step->Custom Process Step*  
* Coloque na caixa de texto **Command** o seguinte valor: **%{CurrentProject:Path}/autorevision.bat**
* Coloque o valor **%{CurrentProject:Path}** na caixa de texto **Working dir**
* Após adicionar o processo manual, clique no botão **Move Up (seta para cima)** para que este processo seja executado antes do qmake
* Pressione CTRL + B para compilar o projeto e CTRL + R para executar o programa

# Plataformas suportadas
* Linux - 32 bits e 64 bits
* Windows - 32 bits e 64 bits

# Captura de Tela
![Imgur](https://i.imgur.com/h0WMkGc.png)