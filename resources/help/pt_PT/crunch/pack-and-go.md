# Pack and Go: enviar um livro com tudo o que precisa
<!-- keywords: pack and go, empacotar, pacote, enviar, partilhar, outro computador, zip, pasta, tipos de letra, fontes, ligações a ficheiros, ligações a imagens, imagens em falta, copiar, levar consigo, unidade USB, licença, ligação externa, livro ligado, células, gráficos -->

Quando envia um livro a alguém, as imagens e os outros livros a que ele está ligado podem perder-se pelo caminho. **Pack and Go** guarda o livro juntamente com esses ficheiros e com os respetivos tipos de letra, num único ficheiro ZIP ou numa única pasta, para que tenha o mesmo aspeto noutro computador.

## Como utilizar

1. Clique em **Ficheiro** e, em seguida, em **Exportar**.
2. Clique em **Pack and Go: Guardar para Outro Computador** para criar um ficheiro ZIP, ou em **Pack and Go: Guardar numa Pasta** para criar uma nova pasta.
3. Escolha onde guardar. Para um ficheiro ZIP, escreva um nome. Para uma pasta, escolha a pasta onde deve ser criada a nova.
4. Leia a mensagem que aparece. Indica onde está o pacote, quantos ficheiros ligados e ficheiros de tipos de letra foram incluídos e tudo o que não pôde ser empacotado.

Um **ficheiro ZIP** é um ficheiro que contém outros ficheiros, pelo que é fácil de enviar. A outra pessoa abre-o para extrair os ficheiros. Uma pasta é mais indicada quando pretende copiar tudo para uma unidade.

## O que entra no pacote

- **O próprio livro**, com o mesmo nome que tem agora. É uma cópia. O seu ficheiro não é alterado.
- **Imagens ligadas.** Uma imagem do livro pode estar ligada ao respetivo ficheiro. O pacote contém uma cópia de cada uma. As imagens inseridas diretamente no livro já estão dentro dele.
- **Livros ligados.** Uma fórmula pode obter números de outro livro. O pacote contém também uma cópia desse livro, e a ligação passa a indicar a cópia. Os tipos de letra são os utilizados nas células, nos gráficos e nas formas.
- **Tipos de letra.** Um tipo de letra é o aspeto das letras. O pacote contém os ficheiros de tipos de letra que acompanham o programa e que o livro utiliza, numa pasta chamada **fonts**. O Pack and Go nunca empacota um tipo de letra cuja licença não permita a sua distribuição. A mensagem indica cada tipo de letra que ficou de fora. Os tipos de letra que instalou no seu computador não são empacotados, pelo que a outra pessoa poderá precisar de os instalar.

## Como as ligações mudam

Na cópia, todas as ligações que foram empacotadas passam a apontar para o ficheiro dentro do pacote, através de um nome curto, como **linked/photo.png**. As ligações deixam de usar o caminho do seu computador, pelo que funcionam onde quer que o pacote seja aberto. Se dois ficheiros tiverem o mesmo nome, ao segundo é acrescentado um número.

Uma ligação que o Pack and Go não consegue seguir mantém-se como estava, e a mensagem indica-a. O Pack and Go não empacota:

- Um ficheiro que não pode ser encontrado.
- Um endereço Web. Estes mantêm-se como estão.
- Um ficheiro noutro computador da sua rede.
- Uma ligação que sobe e sai da pasta do livro, como **../../notas.png**.
- Um documento, livro ou outro objeto que não esteja na pasta do próprio livro (ou numa pasta dentro dela), mesmo quando a ligação indica o caminho completo. Apenas imagens, filmes e sons são empacotados a partir de qualquer local.
- Uma ligação que sai da pasta do livro através de um atalho para outra pasta (uma ligação simbólica).
- Uma ligação para o próprio livro.
- Um ficheiro que não seja uma imagem, um filme, um som ou um documento. Um programa nunca é empacotado.

O Pack and Go não procura mais ligações dentro dos ficheiros ligados, pelo que um ficheiro que aponte de volta para este não pode causar um ciclo.

## Limites

Para evitar que um pacote fique demasiado grande, o Pack and Go pergunta primeiro quando existem mais de 200 ficheiros ligados ou mais de 256 MB deles. Se escolher **Não**, nada é guardado. O Pack and Go nunca empacota mais de 2000 ficheiros nem mais de 1 GB, independentemente da resposta. Os ficheiros que excedam esse limite ficam de fora e são indicados na mensagem.

Se utilizar uma palavra-passe no livro, a cópia no pacote tem a mesma palavra-passe.
