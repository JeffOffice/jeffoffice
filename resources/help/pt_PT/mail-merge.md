# Impressão em série e impressão em série de catálogo
<!-- keywords: impressão em série, unir, destinatários, lista de endereços, etiquetas, cartas modelo, cartas personalizadas, csv, xlsx, vcard, campo de impressão em série, bloco de endereço, catálogo, lista de produtos, impressão em série de correio eletrónico, pré-visualizar, mala direta -->

A impressão em série preenche uma publicação com dados de uma lista. Escreve uma carta uma única vez e o JeffPub cria uma cópia para cada pessoa, com o nome e o endereço dessa pessoa no devido lugar. Uma impressão em série de catálogo repete um esquema para cada item de uma lista.

As ferramentas encontram-se no separador **Correspondência**. Para seguir um percurso guiado, clique em **Correspondência > Iniciar > Impressão em Série > Assistente de Impressão em Série Passo a Passo** para abrir o painel **Impressão em Série**.

## Escolher os destinatários

Clique em **Correspondência > Iniciar > Selecionar Destinatários**:

- **Utilizar uma Lista Existente…**: escolha um ficheiro CSV, um ficheiro de texto separado por tabulações, uma folha de cálculo .xlsx (a primeira folha) ou um ficheiro de contactos .vcf. Num ficheiro CSV, de texto ou de folha de cálculo, a primeira linha tem de conter os nomes das colunas.
- **Selecionar dos Contactos…**: escolha um ficheiro de contactos (.vcf ou .vcard). A maioria dos livros de endereços e telemóveis consegue guardar os contactos num ficheiro deste tipo; cada contacto passa a ser um destinatário, com colunas de nome, endereço, telefone e correio eletrónico.
- **Escrever uma Nova Lista…**: preencha a janela **Nova Lista de Endereços**, utilizando **Nova Entrada** para cada pessoa. Em seguida, o JeffPub propõe guardar a lista como ficheiro CSV.

Para verificar a lista, clique em **Correspondência > Iniciar > Editar Lista de Destinatários**. Na janela **Destinatários da Impressão em Série**, desmarque uma linha para ignorar essa pessoa, clique num cabeçalho para ordenar e escreva em **Filtrar destinatários** para restringir a lista. **Localizar Duplicados** desmarca as repetições e **Validar Endereços** realça as linhas com um endereço em falta ou um código postal estranho.

## Adicionar campos de impressão em série

Um campo de impressão em série é um marcador de posição, como «Nome Próprio», que se transforma num valor real para cada pessoa. Clique numa caixa de texto (se não o fizer, o JeffPub adiciona uma) e, em seguida, utilize **Correspondência > Escrever e Inserir Campos**:

- **Bloco de Endereço**: um endereço postal criado a partir de colunas com nomes como Nome Próprio, Apelido e Linha de Endereço 1.
- **Linha de Saudação**: "Caro" e o nome da pessoa, ou "Caro(a) amigo(a)," se não houver nome.
- **Inserir Campo de Impressão em Série**: escolha qualquer coluna.
- **Campo de Imagem**: escolha uma coluna com nomes de ficheiros de imagem. O JeffPub adiciona uma moldura vazia e procura os ficheiros junto da lista. Depois de guardar e voltar a abrir a publicação, uma imagem cujo ficheiro esteja fora da pasta da publicação só é apresentada quando voltar a escolher a lista de destinatários (**Correspondência > Selecionar Destinatários > Utilizar uma Lista Existente…**). A partir daí, as imagens da pasta da lista, e das pastas dentro dela, são apresentadas até fechar a publicação. O JeffPub não memoriza esta escolha para a vez seguinte. Em alternativa, guarde os ficheiros de imagem na pasta da publicação, ou numa pasta dentro dela, e serão apresentados assim que a publicação for aberta. Isto impede que uma publicação que alguém lhe envie leia, por iniciativa própria, imagens do seu computador. A imagem de cada registo é apresentada quando pré-visualiza os resultados, imprime, guarda um PDF ou une para uma nova publicação ou um catálogo.

## Pré-visualizar os resultados

Clique em **Correspondência > Pré-visualizar Resultados > Pré-visualizar Resultados** para ver os dados reais no lugar dos campos. Utilize **Primeiro Registo**, **Registo Anterior**, **Registo Seguinte** e **Último Registo** para percorrer as pessoas, ou **Localizar Destinatário** para saltar para uma correspondência.

## Concluir a impressão em série

Clique em **Correspondência > Concluir > Concluir e Unir**:

- **Unir para Impressora…** abre a página **Imprimir**. Marque **Imprimir todos os registos da impressão em série** e, em seguida, imprima.
- **Unir para Nova Publicação** cria uma nova publicação com uma cópia das suas páginas para cada pessoa.
- **Unir para PDF…** guarda um único PDF com as páginas de todas as pessoas.
- **Unir para Correio Eletrónico…** cria um ficheiro de correio eletrónico .eml por pessoa, com a primeira página como imagem. Escolha a coluna do correio eletrónico, escreva um assunto e escolha uma pasta. O JeffPub não envia as mensagens. Abra os ficheiros no seu programa de correio eletrónico.
- **Exportar Lista de Destinatários…** guarda a lista como ficheiro CSV.

## Impressão em série de catálogo

Clique em **Inserir > Páginas > Páginas de Catálogo** para abrir o painel **Impressão em Série de Catálogo**.

1. Escolha a lista de produtos, tal como descrito acima.
2. Clique em **Inserir Área de Catálogo**. É apresentada uma caixa com a etiqueta "Área de catálogo". Defina as respetivas **Linhas** e **Colunas**, bem como a posição e o tamanho. Coloque os campos de um item na primeira célula, fazendo duplo clique sobre eles na lista. Os campos repetem-se nas outras células. Para uma imagem, escolha a versão "(imagem)" de um campo.
3. Pré-visualize com as setas. Cada página mostra tantos itens quantas as células da área. Em seguida, una para uma nova publicação, um PDF ou a impressora.

**Remover Área de Catálogo** remove a área.

## Relacionados

- [Imprimir uma publicação](printing)
- [Adicionar e formatar caixas de texto](text-boxes)
- [O separador Correspondência](tab-mailings)
