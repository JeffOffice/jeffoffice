# Impressão em série e impressão em série de catálogo
<!-- keywords: impressão em série, unir, destinatários, lista de endereços, etiquetas, cartas-modelo, csv, xlsx, vcard, campo de impressão em série, bloco de endereço, catálogo, lista de produtos, impressão em série de correio eletrónico, pré-visualizar -->

A impressão em série preenche uma publicação com dados de uma lista. Escreve a carta uma única vez, e o JeffPub cria uma cópia para cada pessoa, com o nome e a morada de cada uma nos lugares certos. Uma impressão em série de catálogo repete um mesmo esquema para cada artigo de uma lista.

As ferramentas estão no separador **Correspondência**. Para seguir um percurso guiado, clique em **Correspondência > Iniciar > Impressão em Série > Assistente de Impressão em Série Passo a Passo** para abrir o painel **Impressão em Série**.

## Escolher os destinatários

Clique em **Correspondência > Iniciar > Selecionar Destinatários**:

- **Utilizar Lista Existente…**: escolha um ficheiro CSV, um ficheiro de texto separado por tabulações, uma folha de cálculo .xlsx (a primeira folha) ou um ficheiro de contactos .vcf. Num ficheiro CSV, de texto ou de folha de cálculo, a primeira linha tem de conter os nomes das colunas.
- **Selecionar dos Contactos…**: escolha um ficheiro de contactos (.vcf ou .vcard). A maioria das agendas e dos telemóveis consegue guardar os contactos num destes formatos; cada contacto torna-se um destinatário, com colunas de nome, morada, telefone e correio eletrónico.
- **Escrever uma Nova Lista…**: preencha a janela **Nova Lista de Endereços**, usando **Nova Entrada** para cada pessoa. Depois, o JeffPub oferece-se para guardar a lista como um ficheiro CSV.

Para verificar a lista, clique em **Correspondência > Iniciar > Editar Lista de Destinatários**. Na janela **Destinatários da Impressão em Série**, desmarque uma linha para excluir essa pessoa, clique num cabeçalho para ordenar e escreva em **Filtrar destinatários** para restringir a lista. **Distinguir os destinatários por** escolhe uma coluna que distinga as pessoas umas das outras; a lista é guardada na sua publicação com as linhas desmarcadas e, quando voltar a escolher o mesmo ficheiro de lista, as mesmas pessoas continuam excluídas, mesmo que tenha ordenado as linhas entretanto. **Localizar Duplicados** desmarca as repetições, e **Validar Endereços** realça as linhas com uma morada em falta ou um código postal estranho.

## Adicionar campos de impressão em série

Um campo de impressão em série é um marcador de posição, como «Nome», que se transforma num valor real para cada pessoa. Clique numa caixa de texto (o JeffPub cria uma se não existir) e use **Correspondência > Escrever e Inserir Campos**:

- **Bloco de Endereço**: uma morada postal construída a partir de colunas com nomes como Nome, Apelido e Morada, linha 1.
- **Linha de Saudação**: "Caro(a)" e o nome da pessoa, ou "Caro amigo(a)," se não houver nome.
- **Inserir Campo de Impressão em Série**: escolha qualquer coluna.
- **Campo de Imagem**: escolha uma coluna com nomes de ficheiros de imagem. O JeffPub adiciona uma moldura vazia e procura os ficheiros junto da sua lista. Depois de guardar a publicação e de a voltar a abrir, uma imagem cujo ficheiro esteja fora da pasta da publicação não aparece até voltar a escolher a lista de destinatários (**Correspondência > Selecionar Destinatários > Utilizar Lista Existente…**). Nesse momento, as imagens da pasta da lista, e das pastas dentro dela, aparecem até fechar a publicação. O JeffPub não guarda isto para a próxima vez. Em alternativa, guarde os ficheiros de imagem na pasta da publicação, ou numa pasta dentro dela, e aparecem assim que a publicação for aberta. Isto evita que uma publicação que lhe enviem leia, por conta própria, imagens do seu computador. A imagem de cada registo aparece ao pré-visualizar os resultados, ao imprimir, ao guardar um PDF ou ao unir para uma nova publicação ou um catálogo.

## Mostrar um número, um preço, uma data ou uma hora à sua maneira

Um campo de impressão em série mostra o valor exatamente como está na lista. Para o alterar, clique junto ao campo (ou selecione-o, ou vários campos) e clique em **Correspondência > Escrever e Inserir Campos > Formatar Campo de Impressão em Série…**. Em **Apresentar o valor como**, escolha:

- **Número**, com 0 a 4 **Casas decimais**. Por exemplo, 1234,5 com 1 casa decimal aparece como 1 234,5.
- **Moeda**, com o símbolo monetário da sua região: 1234,5 aparece como 1 234,50 €, onde esse for o símbolo.
- **Percentagem**, em que 0,256 aparece como 25,6 % com 1 casa decimal.
- **Data**, num **Estilo** como Curto, Longo, Ano, mês, dia, Mês e dia, ou Mês e ano.
- **Hora**, num **Estilo** Curto, de 12 horas ou de 24 horas.

A linha **Amostra** mostra o resultado da sua escolha. Os valores são lidos e apresentados no idioma do texto no campo (inglês dos Estados Unidos, caso não esteja definido nenhum), e não no idioma do seu computador. Em inglês dos Estados Unidos, 03/04/2026 é 4 de março; num idioma em que o dia vem primeiro, como o alemão ou o inglês britânico, é 3 de abril; em alemão, 1.234,5 é um número com vírgula decimal. Um ano de dois dígitos é lido como 2000 a 2029 para 00 a 29, e como 1930 a 1999 para 30 a 99. Um número entre parênteses, como (500), é negativo. Um valor que não seja um número, uma data ou uma hora do tipo escolhido aparece exatamente como está na lista: uma palavra, "nan" ou "inf" (uma pessoa pode chamar-se Nan), um valor vazio, uma hora com um formato de data, ou uma data com um formato de hora. A pré-visualização e a impressão em série final mostram ambas o valor formatado. A escolha conta como um passo do Anular e fica guardada com a publicação.

## Pré-visualizar os resultados

Clique em **Correspondência > Pré-visualizar Resultados > Pré-visualizar Resultados** para ver dados reais no lugar dos campos. Use **Primeiro Registo**, **Registo Anterior**, **Registo Seguinte** e **Último Registo** para percorrer as pessoas, ou **Localizar Destinatário** para saltar para uma correspondência.

## Concluir a impressão em série

Clique em **Correspondência > Concluir > Concluir e Unir**:

- **Unir para Impressora…** abre a página **Imprimir**. Marque **Imprimir todos os registos da impressão em série** e imprima.
- **Unir para Nova Publicação** cria uma nova publicação com uma cópia das suas páginas para cada pessoa.
- **Unir para PDF…** guarda um único PDF com as páginas de todas as pessoas.
- **Unir para Correio Eletrónico…** cria um ficheiro de correio eletrónico .eml por pessoa, com a primeira página como imagem. Escolha a coluna do correio eletrónico, escreva um assunto e escolha uma pasta. O JeffPub não envia as mensagens. Abra os ficheiros no seu programa de correio eletrónico.
- **Guardar Cada um como Ficheiro…** cria uma publicação .jpub por pessoa, numa pasta à sua escolha. Dê nomes aos ficheiros com colunas entre chavetas, como {Apelido}, {Nome}. É-lhe pedida confirmação antes de mais de 200 ficheiros e, uma vez, se já existirem ficheiros com os mesmos nomes. Clique em **Cancelar** para parar entre ficheiros.
- **Exportar Lista de Destinatários…** guarda a lista como um ficheiro CSV.

## Impressão em série de catálogo

Clique em **Inserir > Páginas > Páginas de Catálogo** para abrir o painel **Impressão em Série de Catálogo**.

1. Escolha a lista de produtos, como indicado acima.
2. Clique em **Inserir Área de Catálogo**. Aparece uma caixa com a etiqueta "Área de catálogo". Defina as suas **Linhas** e **Colunas**, bem como a posição e o tamanho. Coloque os campos de um artigo na primeira célula, fazendo duplo clique neles na lista. Repetem-se nas outras células. Escolha a versão "(imagem)" de um campo para uma imagem.
3. Pré-visualize com as setas. Cada página mostra tantos artigos quantas as células da área. Depois, una para uma nova publicação, um PDF ou a impressora.

**Remover Área de Catálogo** elimina a área.

## Relacionados

- [Imprimir uma publicação](printing)
- [Adicionar e formatar caixas de texto](text-boxes)
- [O separador Correspondência](tab-mailings)
