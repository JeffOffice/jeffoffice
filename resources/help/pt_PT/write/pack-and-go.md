# Pack and Go: enviar um documento com tudo o que precisa
<!-- keywords: pack and go, empacotar, pacote, enviar, partilhar, outro computador, zip, pasta, tipos de letra, fontes, ficheiros ligados, imagens ligadas, imagens em falta, copiar, levar consigo, unidade USB, licença, INCLUDEPICTURE, INCLUDETEXT, campo -->

Quando envia um documento a alguém, as imagens e os tipos de letra que ele utiliza podem perder-se pelo caminho. O **Pack and Go** guarda o documento juntamente com o que precisa, num único ficheiro ZIP ou numa única pasta, para que tenha o mesmo aspeto noutro computador.

## Como utilizar

1. Clique em **Ficheiro** e, em seguida, em **Exportar**.
2. Clique em **Pack and Go: Guardar para Outro Computador** para criar um ficheiro ZIP, ou em **Pack and Go: Guardar numa Pasta** para criar uma pasta nova.
3. Escolha onde guardar. Para um ficheiro ZIP, escreva um nome. Para uma pasta, escolha a pasta onde pretende criar a nova.
4. Leia a mensagem que aparece. Indica onde está o pacote, quantos ficheiros ligados e ficheiros de tipos de letra foram incluídos e tudo o que não pôde ser empacotado.

Um **ficheiro ZIP** é um ficheiro que contém outros ficheiros, por isso é fácil de enviar. A outra pessoa abre-o para extrair os ficheiros. Uma pasta é mais indicada quando pretende copiar tudo para uma unidade.

## O que entra no pacote

- **O próprio documento**, com o mesmo nome que tem agora. É uma cópia. O seu ficheiro original não é alterado.
- **Imagens ligadas.** Uma imagem ligada mostra uma imagem de um ficheiro em vez de conter a própria imagem. O pacote guarda uma cópia de cada uma.
- **Ficheiros indicados por campos.** Um campo é um código que apresenta o seu próprio resultado. Um campo **INCLUDEPICTURE** mostra uma imagem de um ficheiro, e um campo **INCLUDETEXT** mostra texto de outro documento. O pacote guarda uma cópia de cada ficheiro, e o campo passa a indicar essa cópia.
- **Tipos de letra.** Um tipo de letra é o aspeto dos caracteres. O pacote guarda os ficheiros de tipos de letra que vêm com o programa e que o documento utiliza, numa pasta chamada **fonts**. O Pack and Go nunca empacota um tipo de letra cuja licença não permita a sua distribuição. A mensagem indica cada tipo de letra que ficou de fora. Os tipos de letra que instalou no seu computador não são empacotados, por isso a outra pessoa poderá precisar deles.

## Como as ligações mudam

Na cópia, cada ligação que foi empacotada aponta para o ficheiro dentro do pacote, através de um nome curto, como **linked/photo.png**. As ligações já não utilizam o caminho no seu computador, por isso funcionam onde quer que o pacote seja aberto. Se dois ficheiros tiverem o mesmo nome, ao segundo é acrescentado um número.

Uma ligação que o Pack and Go não consegue seguir mantém-se como estava, e a mensagem indica-a. O Pack and Go não empacota:

- Um ficheiro que não pode ser encontrado.
- Um endereço Web. Estes mantêm-se como estão.
- Um ficheiro num outro computador da sua rede.
- Uma ligação que sobe e sai da pasta do documento, como **../../notes.png**.
- Um documento, um livro de cálculo ou outro objeto que não esteja na pasta do próprio documento (ou numa pasta dentro dela), mesmo quando a ligação indica o caminho completo. Apenas imagens, vídeos e sons são empacotados a partir de qualquer local.
- Uma ligação que sai da pasta do documento através de um atalho para outra pasta (uma ligação simbólica).
- Uma ligação para o próprio documento.
- Um ficheiro que não seja uma imagem, um vídeo, um som ou um documento. Um programa nunca é empacotado.

O Pack and Go não procura ligações adicionais dentro dos ficheiros ligados, por isso um ficheiro que faça uma ligação de volta a este não pode causar um ciclo.

## Limites

Para evitar que um pacote fique demasiado grande, o Pack and Go pede confirmação quando existem mais de 200 ficheiros ligados, ou mais de 256 MB deles. Se escolher **Não**, nada é guardado. O Pack and Go nunca empacota mais de 2000 ficheiros, nem mais de 1 GB, independentemente da resposta. Os ficheiros que excedam esse limite ficam de fora e são indicados na mensagem.

Se utilizar uma palavra-passe no documento, a cópia incluída no pacote tem a mesma palavra-passe.
